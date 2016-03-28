#include <Catch/catch.hpp>

#include "cad/macro/parser/Parser.h"
#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/ValueProducer.h"

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::callable;
using namespace cad::macro::ast::logic;
using namespace cad::macro::ast::loop;

TEST_CASE("Define") {
  SECTION("EntryFunction") {
    Parser p;
    auto ast = p.parse("def main() {}");
    auto line1 = std::make_shared<std::string>("def main() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun({1, 5, "main", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    Parser p;
    auto ast = p.parse("def fun() {}");
    auto line1 = std::make_shared<std::string>("def fun() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Variable") {
    Parser p;
    auto ast = p.parse("var foo;");
    auto line1 = std::make_shared<std::string>("var foo;");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      Variable var({1, 5, "foo", line1});
      def.definition.emplace(std::move(var));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Parameter") {
    Parser p;
    auto ast = p.parse("def fun(herbert) {}");
    auto line1 = std::make_shared<std::string>("def fun(herbert) {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      Variable var({1, 9, "herbert", line1});
      Scope scope({1, 18, "{", line1});
      Define var_def(var.token);

      fun.parameter.push_back(var);
      var_def.definition.emplace(Variable(var));
      scope.nodes.push_back(std::move(var_def));
      fun.scope = std::make_unique<Scope>(scope);
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Multiple Parameter") {
    Parser p;
    auto ast = p.parse("def fun(herbert, berta) {}");
    auto line1 = std::make_shared<std::string>("def fun(herbert, berta) {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      Variable var1({1, 9, "herbert", line1});
      Variable var2({1, 18, "berta", line1});
      Scope scope({1, 25, "{", line1});
      Define var1_def(var1.token);
      Define var2_def(var2.token);

      fun.parameter.push_back(var1);
      var1_def.definition.emplace(Variable(var1));
      scope.nodes.push_back(std::move(var1_def));
      fun.parameter.push_back(var2);
      var2_def.definition.emplace(Variable(var2));
      scope.nodes.push_back(std::move(var2_def));
      fun.scope = std::make_unique<Scope>(scope);
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("Callable") {
  SECTION("Parameterless") {
    Parser p;
    auto ast = p.parse("fun();");
    auto line1 = std::make_shared<std::string>("fun();");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});

      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Space after function name") {
    Parser p;
    // TODO check message?
    REQUIRE_THROWS(p.parse("fun ();"));
  }

  SECTION("Parameter") {
    Parser p;
    auto ast = p.parse("fun(foo:herbert);");
    auto line1 = std::make_shared<std::string>("fun(foo:herbert);");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});
      Variable var1({1, 5, "foo", line1});
      Variable var2({1, 9, "herbert", line1});

      fun.parameter.emplace_back(var1, var2);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  // SECTION("Parameter") {
  //   Parser p;
  //   auto ast = p.parse("fun(herbert);");

  //   Scope expected({0, 0, ""});
  //   {
  //     Callable fun({1, 1, "fun"});
  //     Variable var1({1, 5, "herbert"});

  //     fun.parameter.push_back(var1);
  //     expected.nodes.push_back(std::move(fun));
  //   }

  //   REQUIRE_THROW(ast == expected);
  // }

  SECTION("Multiple Parameter") {
    Parser p;
    auto ast = p.parse("fun(foo:herbert, bar:berta);");
    auto line1 = std::make_shared<std::string>("fun(foo:herbert, bar:berta);");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});
      Variable var1({1, 5, "foo", line1});
      Variable var2({1, 9, "herbert", line1});
      Variable var3({1, 18, "bar", line1});
      Variable var4({1, 22, "berta", line1});

      fun.parameter.emplace_back(var1, var2);
      fun.parameter.emplace_back(var3, var4);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function Parameter") {
    Parser p;
    auto ast = p.parse("fun(foo:gun());");
    auto line1 = std::make_shared<std::string>("fun(foo:gun());");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});
      Variable var1({1, 5, "foo", line1});
      Callable gun({1, 9, "gun", line1});

      fun.parameter.emplace_back(var1, gun);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Multiple Function Parameter") {
    Parser p;
    auto ast = p.parse("fun(foo:gun(), bar:hun());");
    auto line1 = std::make_shared<std::string>("fun(foo:gun(), bar:hun());");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});
      Variable var1({1, 5, "foo", line1});
      Callable gun({1, 9, "gun", line1});
      Variable var2({1, 16, "bar", line1});
      Callable hun({1, 20, "hun", line1});

      fun.parameter.emplace_back(var1, gun);
      fun.parameter.emplace_back(var2, hun);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("Return") {
  SECTION("Variable") {
    Parser p;
    auto ast = p.parse("return foo;");
    auto line1 = std::make_shared<std::string>("return foo;");

    Scope expected({0, 0, ""});
    {
      Return ret({1, 1, "return", line1});
      Variable foo({1, 8, "foo", line1});

      ret.output = std::make_unique<ValueProducer>(std::move(foo));
      expected.nodes.push_back(std::move(ret));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    Parser p;
    auto ast = p.parse("return fun();");
    auto line1 = std::make_shared<std::string>("return fun();");

    Scope expected({0, 0, ""});
    {
      Return ret({1, 1, "return", line1});
      Callable fun({1, 8, "fun", line1});

      ret.output = std::make_unique<ValueProducer>(std::move(fun));
      expected.nodes.push_back(std::move(ret));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("If") {
  SECTION("Input") {
    SECTION("Variable") {
      Parser p;
      auto ast = p.parse("if(a){}");
      auto line1 = std::make_shared<std::string>("if(a){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        iff.condition =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 6, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Function") {
      Parser p;
      auto ast = p.parse("if(fun()){}");
      auto line1 = std::make_shared<std::string>("if(fun()){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        iff.condition =
            std::make_unique<ValueProducer>(Callable({1, 4, "fun", line1}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
    SECTION("Literal") {
      Parser p;
      auto ast = p.parse("if(true){}");
      auto line1 = std::make_shared<std::string>("if(true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Literal<Literals::BOOL> lit({1, 4, "true", line1});
        lit.data = true;
        iff.condition = std::make_unique<ValueProducer>(lit);
        iff.true_scope = std::make_unique<Scope>(Token(1, 9, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Operator") {
    SECTION("Equal") {
      Parser p;
      auto ast = p.parse("if(a == b){}");
      auto line1 = std::make_shared<std::string>("if(a == b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, "==", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not Equal") {
      Parser p;
      auto ast = p.parse("if(a != b){}");
      auto line1 = std::make_shared<std::string>("if(a != b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, "!=", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        op.operation = BinaryOperation::NOT_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater") {
      Parser p;
      auto ast = p.parse("if(a > b){}");
      auto line1 = std::make_shared<std::string>("if(a > b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, ">", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 8, "b", line1}));
        op.operation = BinaryOperation::GREATER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater Equal") {
      Parser p;
      auto ast = p.parse("if(a >= b){}");
      auto line1 = std::make_shared<std::string>("if(a >= b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, ">=", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        op.operation = BinaryOperation::GREATER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller") {
      Parser p;
      auto ast = p.parse("if(a < b){}");
      auto line1 = std::make_shared<std::string>("if(a < b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, "<", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 8, "b", line1}));
        op.operation = BinaryOperation::SMALLER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller Equal") {
      Parser p;
      auto ast = p.parse("if(a <= b){}");
      auto line1 = std::make_shared<std::string>("if(a <= b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, "<=", line1});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        op.operation = BinaryOperation::SMALLER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not var") {
      Parser p;
      auto ast = p.parse("if(!a){}");
      auto line1 = std::make_shared<std::string>("if(!a){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        UnaryOperator op({1, 4, "!", line1});
        op.operand =
            std::make_unique<ValueProducer>(Variable({1, 5, "a", line1}));
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 7, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not operator") {
      Parser p;
      auto ast = p.parse("if(!(a == b)){}");
      auto line1 = std::make_shared<std::string>("if(!(a == b)){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op2({1, 8, "==", line1});
        op2.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 6, "a", line1}));
        op2.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 11, "b", line1}));
        op2.operation = BinaryOperation::EQUAL;
        UnaryOperator op({1, 4, "!", line1});
        op.operand = std::make_unique<ValueProducer>(op2);
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 14, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Literals") {
    SECTION("Boolean") {
      Parser p;
      auto ast = p.parse("if(true == false){}");
      auto line1 = std::make_shared<std::string>("if(true == false){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, "==", line1});
        Literal<Literals::BOOL> left({1, 4, "true", line1});
        Literal<Literals::BOOL> right({1, 12, "false", line1});
        left.data = true;
        right.data = false;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 18, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Integer") {
      Parser p;
      auto ast = p.parse("if(1 == 1){}");
      auto line1 = std::make_shared<std::string>("if(1 == 1){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 6, "==", line1});
        Literal<Literals::INT> left({1, 4, "1", line1});
        Literal<Literals::INT> right({1, 9, "1", line1});
        left.data = 1;
        right.data = 1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Double") {
      Parser p;
      auto ast = p.parse("if(.1 == .1){}");
      auto line1 = std::make_shared<std::string>("if(.1 == .1){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 7, "==", line1});
        Literal<Literals::DOUBLE> left({1, 4, ".1", line1});
        Literal<Literals::DOUBLE> right({1, 10, ".1", line1});
        left.data = 0.1;
        right.data = 0.1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("String") {
      Parser p;
      // The escaped " counts one!
      auto ast = p.parse("if(\"a\" == \"a\"){}");
      auto line1 = std::make_shared<std::string>("if(\"a\" == \"a\"){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 7, "==", line1});
        Literal<Literals::STRING> left({1, 6, "\"a\"", line1});
        Literal<Literals::STRING> right({1, 12, "\"a\"", line1});
        left.data = "a";
        right.data = "a";
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Combination") {
    SECTION("Brackets") {
      Parser p;
      auto ast = p.parse("if(a == b && (a == c || c == b)){}");
      auto line1 =
          std::make_shared<std::string>("if(a == b && (a == c || c == b)){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator a_eq_c({1, 17, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 15, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 20, "c", line1}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator c_eq_b({1, 27, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 25, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 30, "b", line1}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 22, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        BinaryOperator a_eq_b({1, 6, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        a_eq_b.operation = BinaryOperation::EQUAL;

        BinaryOperator op_and({1, 11, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(op_or);
        op_and.operation = BinaryOperation::AND;

        iff.condition = std::make_unique<ValueProducer>(op_and);
        iff.true_scope = std::make_unique<Scope>(Token(1, 33, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Bracketless") {
      Parser p;
      auto ast = p.parse("if(a == b && a == c || c == b){}");
      auto line1 =
          std::make_shared<std::string>("if(a == b && a == c || c == b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator a_eq_b({1, 6, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b", line1}));
        a_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator a_eq_c({1, 16, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 14, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 19, "c", line1}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator op_and({1, 11, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_and.operation = BinaryOperation::AND;

        BinaryOperator c_eq_b({1, 26, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 24, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 29, "b", line1}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 21, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(op_and);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        iff.condition = std::make_unique<ValueProducer>(op_or);
        iff.true_scope = std::make_unique<Scope>(Token(1, 31, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Else") {
      Parser p;
      auto ast = p.parse("if(a){}else{}");
      auto line1 = std::make_shared<std::string>("if(a){}else{}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        iff.condition =
            std::make_unique<ValueProducer>(Variable({1, 4, "a", line1}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 6, "{", line1));
        iff.false_scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }
}

TEST_CASE("While") {  // Basically an if
  Parser p;
  auto ast = p.parse("while(a){}");
  auto line1 = std::make_shared<std::string>("while(a){}");

  Scope expected({0, 0, ""});
  {
    While w({1, 1, "while", line1});
    w.condition = std::make_unique<ValueProducer>(Variable({1, 7, "a", line1}));
    w.scope = std::make_unique<Scope>(Token(1, 9, "{", line1));
    expected.nodes.push_back(std::move(w));
  }

  REQUIRE(ast == expected);
}

TEST_CASE("\"free\" operators") {
  // TODO
}

TEST_CASE("break") {
  SECTION("While") {
    Parser p;
    auto ast = p.parse("while(a){break;}");
    auto line1 = std::make_shared<std::string>("while(a){break;}");

    Scope expected({0, 0, ""});
    {
      While w({1, 1, "while", line1});
      w.condition =
          std::make_unique<ValueProducer>(Variable({1, 7, "a", line1}));
      w.scope = std::make_unique<Scope>(Token(1, 9, "{", line1));
      w.scope->nodes.emplace_back(Break(Token(1, 10, "break", line1)));
      expected.nodes.push_back(std::move(w));
    }

    REQUIRE(ast == expected);
  }
}

// FIXME
// TEST_CASE("For") {
//   Parser p;
//   auto ast = p.parse("for(var a = 0; a < 10; a = a + 1){}");

//   Scope expected({0, 0, "",line1});
//   {
//     For f({1, 1, "For",line1});
//     f.condition = std::make_unique<ValueProducer>(Variable({1, 4,
//     "a",line1}));


//     f.true_scope = std::make_unique<Scope>(Token(1, 6, "{"));
//     f.false_scope = std::make_unique<Scope>(Token(1, 12, "{"));
//     expected.nodes.push_back(std::move(f));
//   }

//   REQUIRE(ast == expected);
// }


// TEST_CASE("Complete") {
//   const std::string raw_macro = "\n"
//                                 "var a = true;            \n"
//                                 "var b = 2;               \n"
//                                 "var c = \" 3\";          \n"
//                                 "                         \n"
//                                 "                         \n"
//                                 "def fun(foo) {           \n"
//                                 "  var bar;               \n"
//                                 "                         \n"
//                                 "  if(foo == a) {         \n"
//                                 "    bar = foo;           \n"
//                                 "  } else {               \n"
//                                 "    bar = b;             \n"
//                                 "  }                      \n"
//                                 "                         \n"
//                                 "  return bar;            \n"
//                                 "}                        \n"
//                                 "                         \n"
//                                 "                         \n"
//                                 "def main(foo, bar) {     \n"
//                                 "  var baz = foo;         \n"
//                                 "                         \n"
//                                 "  fun(baz);              \n"
//                                 "}                        \n"
//                                 "                         \n";
//   Parser p;
//   WARN(p.parse(raw_macro));
// }
