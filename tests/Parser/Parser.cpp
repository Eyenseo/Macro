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
    auto ast = parse("var herbert; fun(foo:herbert);");
    auto line1 =
        std::make_shared<std::string>("var herbert; fun(foo:herbert);");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "herbert", line1});
      Callable fun({1, 14, "fun", line1});
      Variable var1({1, 18, "foo", line1});
      Variable var2({1, 22, "herbert", line1});

      fun.parameter.emplace_back(var1, var2);
      expected.nodes.push_back(std::move(def));
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Parameter") {
    REQUIRE_THROWS_AS(parse("var herbert; fun(herbert);"),
                      ExceptionBase<UserE>);
  }

  SECTION("Multiple Parameter") {
    auto ast = parse("var herbert; var berta; fun(foo:herbert, bar:berta);");
    auto line1 = std::make_shared<std::string>(
        "var herbert; var berta; fun(foo:herbert, bar:berta);");

    Scope expected({0, 0, ""});
    {
      Define def1({1, 1, "var", line1});
      def1.definition = Variable({1, 5, "herbert", line1});
      Define def2({1, 14, "var", line1});
      def2.definition = Variable({1, 18, "berta", line1});
      Callable fun({1, 25, "fun", line1});
      Variable var1({1, 29, "foo", line1});
      Variable var2({1, 33, "herbert", line1});
      Variable var3({1, 42, "bar", line1});
      Variable var4({1, 46, "berta", line1});


      fun.parameter.emplace_back(var1, var2);
      fun.parameter.emplace_back(var3, var4);
      expected.nodes.push_back(std::move(def1));
      expected.nodes.push_back(std::move(def2));
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
    auto ast = parse("def main() {return 1;}");
    auto line1 = std::make_shared<std::string>("def main() {return 1;}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun({1, 5, "main", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      Return ret({1, 13, "return", line1});
      Literal<Literals::INT> one({1, 20, "1", line1});
      one.data = 1;
      ret.output = std::make_unique<ValueProducer>(std::move(one));
      fun.scope->nodes.push_back(std::move(ret));
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    auto ast = parse("def main() {return fun();}");
    auto line1 = std::make_shared<std::string>("def main() {return fun();}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun1({1, 5, "main", line1});
      fun1.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      Return ret({1, 13, "return", line1});
      Callable fun2({1, 20, "fun", line1});
      ret.output = std::make_unique<ValueProducer>(std::move(fun2));
      fun1.scope->nodes.push_back(std::move(ret));
      def.definition = std::move(fun1);

      expected.nodes.push_back(std::move(def));
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
      auto ast = parse("if(true){}");
      auto line1 = std::make_shared<std::string>("if(true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Literal<Literals::BOOL> lit({1, 4, "true", line1});
        lit.data = true;
        iff.condition = std::make_unique<ValueProducer>(std::move(lit));
        iff.true_scope = std::make_unique<Scope>(Token(1, 9, "{", line1));
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
      auto ast = parse("if(true == true){}");
      auto line1 = std::make_shared<std::string>("if(true == true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, "==", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not Equal") {
      auto ast = parse("if(true != true){}");
      auto line1 = std::make_shared<std::string>("if(true != true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, "!=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::NOT_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater") {
      auto ast = parse("if(true > true){}");
      auto line1 = std::make_shared<std::string>("if(true > true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, ">", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 11, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::GREATER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater Equal") {
      auto ast = parse("if(true >= true){}");
      auto line1 = std::make_shared<std::string>("if(true >= true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, ">=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::GREATER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller") {
      auto ast = parse("if(true < true){}");
      auto line1 = std::make_shared<std::string>("if(true < true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, "<", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 11, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::SMALLER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller Equal") {
      auto ast = parse("if(true <= true){}");
      auto line1 = std::make_shared<std::string>("if(true <= true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op({1, 9, "<=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = BinaryOperation::SMALLER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not var") {
      auto ast = parse("if(!true){}");
      auto line1 = std::make_shared<std::string>("if(!true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        UnaryOperator op({1, 4, "!", line1});
        Literal<Literals::BOOL> lit({1, 5, "true", line1});
        lit.data = true;
        op.operand = std::make_unique<ValueProducer>(std::move(lit));
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not operator") {
      auto ast = parse("if(!(true == true)){}");
      auto line1 = std::make_shared<std::string>("if(!(true == true)){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        BinaryOperator op2({1, 11, "==", line1});
        Literal<Literals::BOOL> lit1({1, 6, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 14, "true", line1});
        lit2.data = true;
        op2.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op2.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op2.operation = BinaryOperation::EQUAL;
        UnaryOperator op({1, 4, "!", line1});
        op.operand = std::make_unique<ValueProducer>(op2);
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 20, "{", line1));
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
      auto ast = parse("var a;var b;var c;if(a == b && (a == c || c == b)){}");
      auto line1 = std::make_shared<std::string>(
          "var a;var b;var c;if(a == b && (a == c || c == b)){}");

      Scope expected({0, 0, ""});
      {
        Define def1({1, 1, "var", line1});
        def1.definition = Variable({1, 5, "a", line1});
        Define def2({1, 7, "var", line1});
        def2.definition = Variable({1, 11, "b", line1});
        Define def3({1, 13, "var", line1});
        def3.definition = Variable({1, 17, "c", line1});

        If iff({1, 19, "if", line1});
        BinaryOperator a_eq_c({1, 35, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 33, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 38, "c", line1}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator c_eq_b({1, 45, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 43, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 48, "b", line1}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 40, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        BinaryOperator a_eq_b({1, 24, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 22, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 27, "b", line1}));
        a_eq_b.operation = BinaryOperation::EQUAL;

        BinaryOperator op_and({1, 29, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(op_or);
        op_and.operation = BinaryOperation::AND;

        iff.condition = std::make_unique<ValueProducer>(op_and);
        iff.true_scope = std::make_unique<Scope>(Token(1, 51, "{", line1));
        expected.nodes.push_back(std::move(def1));
        expected.nodes.push_back(std::move(def2));
        expected.nodes.push_back(std::move(def3));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Bracketless") {
      auto ast = parse("var a;var b;var c;if(a == b && a == c || c == b){}");
      auto line1 = std::make_shared<std::string>(
          "var a;var b;var c;if(a == b && a == c || c == b){}");

      Scope expected({0, 0, ""});
      {
        Define def1({1, 1, "var", line1});
        def1.definition = Variable({1, 5, "a", line1});
        Define def2({1, 7, "var", line1});
        def2.definition = Variable({1, 11, "b", line1});
        Define def3({1, 13, "var", line1});
        def3.definition = Variable({1, 17, "c", line1});

        If iff({1, 19, "if", line1});
        BinaryOperator a_eq_b({1, 24, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 22, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 27, "b", line1}));
        a_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator a_eq_c({1, 34, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 32, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 37, "c", line1}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator op_and({1, 29, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_and.operation = BinaryOperation::AND;

        BinaryOperator c_eq_b({1, 44, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 42, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 47, "b", line1}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 39, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(op_and);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        iff.condition = std::make_unique<ValueProducer>(op_or);
        iff.true_scope = std::make_unique<Scope>(Token(1, 49, "{", line1));
        expected.nodes.push_back(std::move(def1));
        expected.nodes.push_back(std::move(def2));
        expected.nodes.push_back(std::move(def3));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Else") {
      auto ast = parse("var a; if(a){}else{}");
      auto line1 = std::make_shared<std::string>("var a; if(a){}else{}");

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "var", line1});
        def.definition = Variable({1, 5, "a", line1});
        If iff({1, 8, "if", line1});
        iff.condition =
            std::make_unique<ValueProducer>(Variable({1, 11, "a", line1}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{", line1));
        iff.false_scope = std::make_unique<Scope>(Token(1, 19, "{", line1));
        expected.nodes.push_back(std::move(def));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }
}

TEST_CASE("While") {  // Basically an if
  SECTION("default") {
    auto ast = parse("var a; while(a){}");
    auto line1 = std::make_shared<std::string>("var a; while(a){}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "a", line1});
      While w({1, 8, "while", line1});
      w.condition =
          std::make_unique<ValueProducer>(Variable({1, 14, "a", line1}));
      w.scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
      expected.nodes.push_back(std::move(def));
      expected.nodes.push_back(std::move(w));
    }

    REQUIRE(ast == expected);
  }
}
TEST_CASE("DoWhile") {
  SECTION("default") {
    auto ast = parse("var a; do{}while(a);");
    auto line1 = std::make_shared<std::string>("var a; do{}while(a);");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "a", line1});
      DoWhile w({1, 8, "do", line1});
      w.condition =
          std::make_unique<ValueProducer>(Variable({1, 18, "a", line1}));
      w.scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
      expected.nodes.push_back(std::move(def));
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
      auto ast = parse("var foo; foo = foo + 1;");
      auto line1 = std::make_shared<std::string>("var foo; foo = foo + 1;");

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "var", line1});
        def.definition = Variable({1, 5, "foo", line1});
        BinaryOperator op_as({1, 14, "=", line1});
        BinaryOperator op_ad({1, 20, "+", line1});
        Literal<Literals::INT> right({1, 22, "1", line1});
        right.data = 1;

        op_ad.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 16, "foo", line1}));
        op_ad.right_operand = std::make_unique<ValueProducer>(right);
        op_ad.operation = BinaryOperation::ADD;

        op_as.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 10, "foo", line1}));
        op_as.right_operand = std::make_unique<ValueProducer>(op_ad);
        op_as.operation = BinaryOperation::ASSIGNMENT;

        expected.nodes.push_back(std::move(def));
        expected.nodes.push_back(std::move(op_as));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Missing right operand") {
      REQUIRE_THROWS_AS(parse("var foo; foo = foo + ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing operator") {
      REQUIRE_THROWS_AS(parse("var foo; foo = foo  1;"), ExceptionBase<UserE>);
    }
    SECTION("Missing left operand") {
      REQUIRE_THROWS_AS(parse("var foo; foo =  + 1;"), ExceptionBase<UserE>);
    }
    SECTION("Missing both operands") {
      REQUIRE_THROWS_AS(parse("var foo; foo =  + ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing everything") {
      REQUIRE_THROWS_AS(parse("var foo; foo =  ;"), ExceptionBase<UserE>);
    }
    SECTION("Missing everything and var") {
      REQUIRE_THROWS_AS(parse("var foo; foo =  ; foo"), ExceptionBase<UserE>);
    }

    SECTION("Missing semicolon") {
      REQUIRE_THROWS_AS(parse("var foo; foo = foo + 1"), ExceptionBase<UserE>);

      SECTION("Missing right operand") {
        REQUIRE_THROWS_AS(parse("var foo; foo = foo + "), ExceptionBase<UserE>);
      }
      SECTION("Missing operator") {
        REQUIRE_THROWS_AS(parse("var foo; foo = foo  1"), ExceptionBase<UserE>);
      }
      SECTION("Missing left operand") {
        REQUIRE_THROWS_AS(parse("var foo; foo =  + 1"), ExceptionBase<UserE>);
      }
      SECTION("Missing both operands") {
        REQUIRE_THROWS_AS(parse("var foo; foo =  +"), ExceptionBase<UserE>);
      }
      SECTION("Missing everything") {
        REQUIRE_THROWS_AS(parse("var foo; foo = "), ExceptionBase<UserE>);
      }
    }
  }
}

TEST_CASE("break") {
  SECTION("While") {
    auto ast = parse("var a; while(a){break;}");
    auto line1 = std::make_shared<std::string>("var a; while(a){break;}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "a", line1});
      While w({1, 8, "while", line1});
      w.condition =
          std::make_unique<ValueProducer>(Variable({1, 14, "a", line1}));
      w.scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
      w.scope->nodes.emplace_back(Break(Token(1, 17, "break", line1)));
      expected.nodes.push_back(std::move(def));
      expected.nodes.push_back(std::move(w));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("return in root") {
  REQUIRE_THROWS_AS(parse("return 1;"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("{return 1;}"), ExceptionBase<UserE>);
}
TEST_CASE("return and break last") {
  SECTION("return") {
    REQUIRE_THROWS_AS(parse("def main(){return 1; 1 + 1;}"),
                      ExceptionBase<UserE>);
  }
  SECTION("return") {
    REQUIRE_THROWS_AS(parse("def main(){while(true){break; 1;}}"),
                      ExceptionBase<UserE>);
  }
}
TEST_CASE("unique parameter") {
  SECTION("call") {
    REQUIRE_THROWS_AS(parse("fun(a:1, a:1); def main(){}"),
                      ExceptionBase<UserE>);
  }
  SECTION("function") {
    REQUIRE_THROWS_AS(parse("def fun(a, a){} def main(){}"),
                      ExceptionBase<UserE>);
  }
  SECTION("main") {
    REQUIRE_THROWS_AS(parse("def main(a,a){}"), ExceptionBase<UserE>);
  }
}
TEST_CASE("unique main") {
  REQUIRE_THROWS_AS(parse("def main(a){} def main(){}"), ExceptionBase<UserE>);
}
TEST_CASE("main only in root") {
  REQUIRE_THROWS_AS(parse("def fun(){def main(a){}}"), ExceptionBase<UserE>);
}
TEST_CASE("unknown variable") {
  REQUIRE_THROWS_AS(parse("def main(){a + 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){if(a){}}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){fun(a);}"), ExceptionBase<UserE>);
}
TEST_CASE("double def") {
  SECTION("variable") {
    REQUIRE_THROWS_AS(parse("var a; var a; def main(){}"),
                      ExceptionBase<UserE>);
  }
  SECTION("function") {
    REQUIRE_THROWS_AS(parse("def fun(){} def fun(){}def main(){}"),
                      ExceptionBase<UserE>);
    REQUIRE_THROWS_AS(parse("def fun(a,b){} def fun(b,a){}def main(){}"),
                      ExceptionBase<UserE>);
  }
}
TEST_CASE("missing operand") {
  SECTION("unary") {
    REQUIRE_THROWS_AS(parse("def main(){!;}"), ExceptionBase<UserE>);
  }
  SECTION("binary") {
    REQUIRE_THROWS_AS(parse("def main(){1 ==;}"), ExceptionBase<UserE>);
    REQUIRE_THROWS_AS(parse("def main(){== 1;}"), ExceptionBase<UserE>);
    REQUIRE_THROWS_AS(parse("def main(){==;}"), ExceptionBase<UserE>);
  }
}
TEST_CASE("assign to non variable") {
  REQUIRE_THROWS_AS(parse("def main(){ 1 = 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){ 1.0 = 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){ \"1\" = 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){ true = 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){ false = 1;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){ fun() = 1;}"), ExceptionBase<UserE>);
}
TEST_CASE("missing scope") {
  REQUIRE_THROWS_AS(parse("def main()"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){} def fun "), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){if(true)}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){if(true){}else}"), ExceptionBase<UserE>);
  // REQUIRE_THROWS_AS(parse("def main(){do while(true);}"),
  // ExceptionBase<UserE>); // FIXME
  // REQUIRE_THROWS_AS(parse("def main(){do;}"), ExceptionBase<UserE>); // FIXME
  REQUIRE_THROWS_AS(parse("def main(){while(true)}"), ExceptionBase<UserE>);
}
TEST_CASE("missing condition") {
  REQUIRE_THROWS_AS(parse("def main(){if(){}}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){if(){}else{}}"), ExceptionBase<UserE>);
  // REQUIRE_THROWS_AS(parse("def main(){do{}while();}"), ExceptionBase<UserE>);
  // // FIXME
  REQUIRE_THROWS_AS(parse("def main(){while(){}}"), ExceptionBase<UserE>);
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
