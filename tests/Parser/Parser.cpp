#include <Catch/catch.hpp>

#include "cad/macro/parser/Parser.h"
#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/ValueProducer.h"

#include <exception.h>

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::callable;
using namespace cad::macro::ast::logic;
using namespace cad::macro::ast::loop;

CATCH_TRANSLATE_EXCEPTION(std::exception& e) {
  std::stringstream ss;
  exception::print_exception(e, ss);
  return ss.str();
}

TEST_CASE("Define") {
  SECTION("EntryFunction") {
    auto ast = parse("def main() {}");
    auto line1 = std::make_shared<std::string>("def main() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun({1, 5, "main", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    auto ast = parse("def fun() {}");
    auto line1 = std::make_shared<std::string>("def fun() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Variable") {
    auto ast = parse("var foo;");
    auto line1 = std::make_shared<std::string>("var foo;");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      Variable var({1, 5, "foo", line1});
      def.definition = std::move(var);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Parameter") {
    auto ast = parse("def fun(herbert) {}");
    auto line1 = std::make_shared<std::string>("def fun(herbert) {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      Variable var({1, 9, "herbert", line1});
      Scope scope({1, 18, "{", line1});

      fun.parameter.push_back(var);
      fun.scope = std::make_unique<Scope>(scope);
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Multiple Parameter") {
    auto ast = parse("def fun(herbert, berta) {}");
    auto line1 = std::make_shared<std::string>("def fun(herbert, berta) {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      Function fun({1, 5, "fun", line1});
      Variable var1({1, 9, "herbert", line1});
      Variable var2({1, 18, "berta", line1});
      Scope scope({1, 25, "{", line1});

      fun.parameter.push_back(var1);
      fun.parameter.push_back(var2);
      fun.scope = std::make_unique<Scope>(scope);
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("Callable") {
  SECTION("Parameterless") {
    auto ast = parse("fun();");
    auto line1 = std::make_shared<std::string>("fun();");

    Scope expected({0, 0, ""});
    {
      Callable fun({1, 1, "fun", line1});

      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Space after function name") {
    REQUIRE_THROWS_AS(parse("fun ();"), ExceptionBase<UserE>);
  }

  SECTION("Parameter") {
    auto ast = parse("fun(foo:herbert);");
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

  SECTION("Parameter") {
    REQUIRE_THROWS_AS(parse("fun(herbert);"), ExceptionBase<UserE>);
  }

  SECTION("Multiple Parameter") {
    auto ast = parse("fun(foo:herbert, bar:berta);");
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
    auto ast = parse("fun(foo:gun());");
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
    auto ast = parse("fun(foo:gun(), bar:hun());");
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
    auto ast = parse("return foo;");
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
    auto ast = parse("return fun();");
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

  SECTION("Function") {
    // TODO better error message
    REQUIRE_THROWS_AS(parse("retrun fun();"), ExceptionBase<UserE>);
    //                          ^
  }
}

TEST_CASE("If") {
  SECTION("Input") {
    SECTION("Variable") {
      auto ast = parse("if(a){}");
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
      auto ast = parse("if(fun()){}");
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
      auto ast = parse("if(true){}");
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
      auto ast = parse("if(a == b){}");
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
      auto ast = parse("if(a != b){}");
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
      auto ast = parse("if(a > b){}");
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
      auto ast = parse("if(a >= b){}");
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
      auto ast = parse("if(a < b){}");
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
      auto ast = parse("if(a <= b){}");
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
      auto ast = parse("if(!a){}");
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
      auto ast = parse("if(!(a == b)){}");
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

    SECTION("Missing Operator") {
      REQUIRE_THROWS_AS(parse("if(a b){}"), ExceptionBase<UserE>);
    }
  }

  SECTION("Literals") {
    SECTION("Boolean") {
      auto ast = parse("if(true == false){}");
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
      auto ast = parse("if(1 == 1){}");
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
      auto ast = parse("if(.1 == .1){}");
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
      // The escaped " counts one!
      auto ast = parse("if(\"a\" == \"a\"){}");
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
      auto ast = parse("if(a == b && (a == c || c == b)){}");
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
      auto ast = parse("if(a == b && a == c || c == b){}");
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
      auto ast = parse("if(a){}else{}");
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
  SECTION("default") {
    auto ast = parse("while(a){}");
    auto line1 = std::make_shared<std::string>("while(a){}");

    Scope expected({0, 0, ""});
    {
      While w({1, 1, "while", line1});
      w.condition =
          std::make_unique<ValueProducer>(Variable({1, 7, "a", line1}));
      w.scope = std::make_unique<Scope>(Token(1, 9, "{", line1));
      expected.nodes.push_back(std::move(w));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("Variable define and assign") {
  SECTION("define and assign") {
    auto ast = parse("var foo = foo + 1;");
    auto line1 = std::make_shared<std::string>("var foo = foo + 1;");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "foo", line1});


      BinaryOperator op_as({1, 9, "=", line1});
      BinaryOperator op_ad({1, 15, "+", line1});
      Literal<Literals::INT> right({1, 17, "1", line1});
      right.data = 1;

      op_ad.left_operand =
          std::make_unique<ValueProducer>(Variable({1, 11, "foo", line1}));
      op_ad.right_operand = std::make_unique<ValueProducer>(right);
      op_ad.operation = BinaryOperation::ADD;

      op_as.left_operand =
          std::make_unique<ValueProducer>(Variable({1, 5, "foo", line1}));
      op_as.right_operand = std::make_unique<ValueProducer>(op_ad);
      op_as.operation = BinaryOperation::ASSIGNMENT;

      expected.nodes.push_back(std::move(def));
      expected.nodes.push_back(std::move(op_as));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Missing right operand") {
    REQUIRE_THROWS_AS(parse("var foo = foo + ;"), ExceptionBase<UserE>);
  }
  SECTION("Missing operator") {
    REQUIRE_THROWS_AS(parse("var foo = foo  1;"), ExceptionBase<UserE>);
  }
  SECTION("Missing left operand") {
    REQUIRE_THROWS_AS(parse("var foo =  + 1;"), ExceptionBase<UserE>);
  }
  SECTION("Missing both operands") {
    REQUIRE_THROWS_AS(parse("var foo =  + ;"), ExceptionBase<UserE>);
  }
  SECTION("Missing everything") {
    REQUIRE_THROWS_AS(parse("var foo =  ;"), ExceptionBase<UserE>);
  }
  SECTION("Missing everything and var") {
    REQUIRE_THROWS_AS(parse("var foo =  ; foo"), ExceptionBase<UserE>);
  }

  SECTION("Missing semicolon") {
    REQUIRE_THROWS_AS(parse("var foo = foo + 1"), ExceptionBase<UserE>);

    SECTION("Missing right operand") {
      REQUIRE_THROWS_AS(parse("var foo = foo + "), ExceptionBase<UserE>);
    }
    SECTION("Missing operator") {
      REQUIRE_THROWS_AS(parse("var foo = foo  1"), ExceptionBase<UserE>);
    }
    SECTION("Missing left operand") {
      REQUIRE_THROWS_AS(parse("var foo =  + 1"), ExceptionBase<UserE>);
    }
    SECTION("Missing both operands") {
      REQUIRE_THROWS_AS(parse("var foo =  +"), ExceptionBase<UserE>);
    }
    SECTION("Missing everything") {
      REQUIRE_THROWS_AS(parse("var foo =  "), ExceptionBase<UserE>);
    }
  }
}

TEST_CASE("\"free\" operators") {
  SECTION("free assign") {
    SECTION("define and assign") {
      auto ast = parse("foo = foo + 1;");
      auto line1 = std::make_shared<std::string>("foo = foo + 1;");

      Scope expected({0, 0, ""});
      {
        BinaryOperator op_as({1, 5, "=", line1});
        BinaryOperator op_ad({1, 11, "+", line1});
        Literal<Literals::INT> right({1, 13, "1", line1});
        right.data = 1;

        op_ad.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 7, "foo", line1}));
        op_ad.right_operand = std::make_unique<ValueProducer>(right);
        op_ad.operation = BinaryOperation::ADD;

        op_as.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 1, "foo", line1}));
        op_as.right_operand = std::make_unique<ValueProducer>(op_ad);
        op_as.operation = BinaryOperation::ASSIGNMENT;

        expected.nodes.push_back(std::move(op_as));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Missing right operand") {
      REQUIRE_THROWS_AS(parse("foo = foo + ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing operator") {
      REQUIRE_THROWS_AS(parse("foo = foo  1;"), ExceptionBase<UserE>);
    }
    SECTION("Missing left operand") {
      REQUIRE_THROWS_AS(parse("foo =  + 1;"), ExceptionBase<UserE>);
    }
    SECTION("Missing both operands") {
      REQUIRE_THROWS_AS(parse("foo =  + ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing everything") {
      REQUIRE_THROWS_AS(parse("foo =  ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing everything and var") {
      REQUIRE_THROWS_AS(parse("foo =  ; foo"), ExceptionBase<UserE>);
    }

    SECTION("Missing semicolon") {
      REQUIRE_THROWS_AS(parse("foo = foo + 1"), ExceptionBase<UserE>);

      SECTION("Missing right operand") {
        REQUIRE_THROWS_AS(parse("foo = foo + "), ExceptionBase<UserE>);
      }
      SECTION("Missing operator") {
        REQUIRE_THROWS_AS(parse("foo = foo  1"), ExceptionBase<UserE>);
      }
      SECTION("Missing left operand") {
        REQUIRE_THROWS_AS(parse("foo =  + 1"), ExceptionBase<UserE>);
      }
      SECTION("Missing both operands") {
        REQUIRE_THROWS_AS(parse("foo =  +"), ExceptionBase<UserE>);
      }
      SECTION("Missing everything") {
        REQUIRE_THROWS_AS(parse("foo = "), ExceptionBase<UserE>);
      }
    }
  }
}

TEST_CASE("break") {
  SECTION("While") {
    auto ast = parse("while(a){break;}");
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


// TODO
// TEST_CASE("For") {
//   auto ast = parse("for(var a = 0; a < 10; a = a + 1){}");

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
//                                 "var a = true;              \n"
//                                 "var b = 2;                 \n"
//                                 "var c = \" 3\";            \n"
//                                 "                           \n"
//                                 "                           \n"
//                                 "def fun(foo) {             \n"
//                                 "  var bar;                 \n"
//                                 "                           \n"
//                                 "  def gun() {              \n"
//                                 "    var i = 0;             \n"
//                                 "                           \n"
//                                 "    while (i < 3) {        \n"
//                                 "      i = i + 1;           \n"
//                                 "    }                      \n"
//                                 "    return i;              \n"
//                                 "  }                        \n"
//                                 "                           \n"
//                                 "  {                        \n"
//                                 "    var bar;               \n"
//                                 "  }                        \n"
//                                 "                           \n"
//                                 "  if(foo == a) {           \n"
//                                 "    bar = foo;             \n"
//                                 "  } else {                 \n"
//                                 "    bar = gun();           \n"
//                                 "  }                        \n"
//                                 "                           \n"
//                                 "  return bar;              \n"
//                                 "}                          \n"
//                                 "                           \n"
//                                 "                           \n"
//                                 "def main(foo, bar) {       \n"
//                                 "  var baz = foo;           \n"
//                                 "                           \n"
//                                 "  fun(foo:baz);            \n"
//                                 "}                          \n";
//   WARN(parse(raw_macro));
// }
