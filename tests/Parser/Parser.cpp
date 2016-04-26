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
    auto line1 = std::make_shared<std::string>("def main() {}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("def fun() {}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("var foo;");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("def fun(herbert) {}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("def fun(herbert, berta) {}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("fun();");
    auto ast = parse(*line1);

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
    auto line1 =
        std::make_shared<std::string>("var herbert; fun(foo:herbert);");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>(
        "var herbert; var berta; fun(foo:herbert, bar:berta);");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("fun(foo:gun());");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("fun(foo:gun(), bar:hun());");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("def main() {return 1;}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("def main() {return fun();}");
    auto ast = parse(*line1);

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
      auto line1 = std::make_shared<std::string>("if(true){}");
      auto ast = parse(*line1);

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
      auto line1 = std::make_shared<std::string>("if(fun()){}");
      auto ast = parse(*line1);

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
      auto line1 = std::make_shared<std::string>("if(true){}");
      auto ast = parse(*line1);

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
      auto line1 = std::make_shared<std::string>("if(true == true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, "==", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not Equal") {
      auto line1 = std::make_shared<std::string>("if(true != true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, "!=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::NOT_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater") {
      auto line1 = std::make_shared<std::string>("if(true > true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, ">", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 11, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::GREATER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater Equal") {
      auto line1 = std::make_shared<std::string>("if(true >= true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, ">=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::GREATER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller") {
      auto line1 = std::make_shared<std::string>("if(true < true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, "<", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 11, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::SMALLER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 16, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller Equal") {
      auto line1 = std::make_shared<std::string>("if(true <= true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, "<=", line1});
        Literal<Literals::BOOL> lit1({1, 4, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 12, "true", line1});
        lit2.data = true;

        op.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op.operation = Operation::SMALLER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 17, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not var") {
      auto line1 = std::make_shared<std::string>("if(!true){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 4, "!", line1});
        Literal<Literals::BOOL> lit({1, 5, "true", line1});
        lit.data = true;
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit));
        op.operation = Operation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not operator") {
      auto line1 = std::make_shared<std::string>("if(!(true == true)){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op2({1, 11, "==", line1});
        Literal<Literals::BOOL> lit1({1, 6, "true", line1});
        lit1.data = true;
        Literal<Literals::BOOL> lit2({1, 14, "true", line1});
        lit2.data = true;
        op2.left_operand = std::make_unique<ValueProducer>(std::move(lit1));
        op2.right_operand = std::make_unique<ValueProducer>(std::move(lit2));
        op2.operation = Operation::EQUAL;
        Operator op({1, 4, "!", line1});
        op.right_operand = std::make_unique<ValueProducer>(op2);
        op.operation = Operation::NOT;

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
      auto line1 = std::make_shared<std::string>("if(true == false){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 9, "==", line1});
        Literal<Literals::BOOL> left({1, 4, "true", line1});
        Literal<Literals::BOOL> right({1, 12, "false", line1});
        left.data = true;
        right.data = false;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = Operation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 18, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Integer") {
      auto line1 = std::make_shared<std::string>("if(1 == 1){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 6, "==", line1});
        Literal<Literals::INT> left({1, 4, "1", line1});
        Literal<Literals::INT> right({1, 9, "1", line1});
        left.data = 1;
        right.data = 1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = Operation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Double") {
      auto line1 = std::make_shared<std::string>("if(.1 == .1){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 7, "==", line1});
        Literal<Literals::DOUBLE> left({1, 4, ".1", line1});
        Literal<Literals::DOUBLE> right({1, 10, ".1", line1});
        left.data = 0.1;
        right.data = 0.1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = Operation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("String") {
      // The escaped " counts one!
      auto line1 = std::make_shared<std::string>("if(\"a\" == \"a\"){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if", line1});
        Operator op({1, 8, "==", line1});
        Literal<Literals::STRING> left({1, 4, "\"a\"", line1});
        Literal<Literals::STRING> right({1, 11, "\"a\"", line1});
        left.data = "a";
        right.data = "a";
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = Operation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 15, "{", line1));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Combination") {
    SECTION("Brackets") {
      auto line1 = std::make_shared<std::string>(
          "var a;var b;var c;if(a == b && (a == c || c == b)){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def1({1, 1, "var", line1});
        def1.definition = Variable({1, 5, "a", line1});
        Define def2({1, 7, "var", line1});
        def2.definition = Variable({1, 11, "b", line1});
        Define def3({1, 13, "var", line1});
        def3.definition = Variable({1, 17, "c", line1});

        If iff({1, 19, "if", line1});
        Operator a_eq_c({1, 35, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 33, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 38, "c", line1}));
        a_eq_c.operation = Operation::EQUAL;
        Operator c_eq_b({1, 45, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 43, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 48, "b", line1}));
        c_eq_b.operation = Operation::EQUAL;
        Operator op_or({1, 40, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = Operation::OR;

        Operator a_eq_b({1, 24, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 22, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 27, "b", line1}));
        a_eq_b.operation = Operation::EQUAL;

        Operator op_and({1, 29, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(op_or);
        op_and.operation = Operation::AND;

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
      auto line1 = std::make_shared<std::string>(
          "var a;var b;var c;if(a == b && a == c || c == b){}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def1({1, 1, "var", line1});
        def1.definition = Variable({1, 5, "a", line1});
        Define def2({1, 7, "var", line1});
        def2.definition = Variable({1, 11, "b", line1});
        Define def3({1, 13, "var", line1});
        def3.definition = Variable({1, 17, "c", line1});

        If iff({1, 19, "if", line1});
        Operator a_eq_b({1, 24, "==", line1});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 22, "a", line1}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 27, "b", line1}));
        a_eq_b.operation = Operation::EQUAL;
        Operator a_eq_c({1, 34, "==", line1});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 32, "a", line1}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 37, "c", line1}));
        a_eq_c.operation = Operation::EQUAL;
        Operator op_and({1, 29, "&&", line1});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_and.operation = Operation::AND;

        Operator c_eq_b({1, 44, "==", line1});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 42, "c", line1}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 47, "b", line1}));
        c_eq_b.operation = Operation::EQUAL;
        Operator op_or({1, 39, "||", line1});
        op_or.left_operand = std::make_unique<ValueProducer>(op_and);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = Operation::OR;

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
      auto line1 = std::make_shared<std::string>("var a; if(a){}else{}");
      auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("var a; while(a){}");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("var a; do{}while(a);");
    auto ast = parse(*line1);

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
    auto line1 = std::make_shared<std::string>("var foo = foo + 1;");
    auto ast = parse(*line1);

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var", line1});
      def.definition = Variable({1, 5, "foo", line1});


      Operator op_as({1, 9, "=", line1});
      Operator op_ad({1, 15, "+", line1});
      Literal<Literals::INT> right({1, 17, "1", line1});
      right.data = 1;

      op_ad.left_operand =
          std::make_unique<ValueProducer>(Variable({1, 11, "foo", line1}));
      op_ad.right_operand = std::make_unique<ValueProducer>(right);
      op_ad.operation = Operation::ADD;

      op_as.left_operand =
          std::make_unique<ValueProducer>(Variable({1, 5, "foo", line1}));
      op_as.right_operand = std::make_unique<ValueProducer>(op_ad);
      op_as.operation = Operation::ASSIGNMENT;

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
    REQUIRE_NOTHROW(parse("var foo =  + 1;"));  // This is valid!
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
      auto line1 = std::make_shared<std::string>("var foo; foo = foo + 1;");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "var", line1});
        def.definition = Variable({1, 5, "foo", line1});
        Operator op_as({1, 14, "=", line1});
        Operator op_ad({1, 20, "+", line1});
        Literal<Literals::INT> right({1, 22, "1", line1});
        right.data = 1;

        op_ad.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 16, "foo", line1}));
        op_ad.right_operand = std::make_unique<ValueProducer>(right);
        op_ad.operation = Operation::ADD;

        op_as.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 10, "foo", line1}));
        op_as.right_operand = std::make_unique<ValueProducer>(op_ad);
        op_as.operation = Operation::ASSIGNMENT;

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
      REQUIRE_NOTHROW(parse("var foo; foo =  + 1;"));  // This is valid!
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
    auto line1 = std::make_shared<std::string>("var a; while(a){break;}");
    auto ast = parse(*line1);

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

TEST_CASE("Number Literals") {
  SECTION("Integer") {
    SECTION("positive") {
      auto line1 = std::make_shared<std::string>("def main() {return 1;}");
      auto ast = parse(*line1);

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
    SECTION("negative") {
      auto line1 = std::make_shared<std::string>("def main() {return -1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 20, "-", line1});
        Literal<Literals::INT> one({1, 21, "1", line1});
        one.data = 1;
        op.right_operand = std::make_unique<ValueProducer>(std::move(one));
        op.operation = Operation::NEGATIVE;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negative operator") {
      auto line1 = std::make_shared<std::string>("def main() {return 1 - 1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 22, "-", line1});
        Literal<Literals::INT> one1({1, 20, "1", line1});
        Literal<Literals::INT> one2({1, 24, "1", line1});
        one1.data = 1;
        one2.data = 1;
        op.left_operand = std::make_unique<ValueProducer>(std::move(one1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::SUBTRACT;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negative operator nospace") {
      auto line1 = std::make_shared<std::string>("def main() {return 1-1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 21, "-", line1});
        Literal<Literals::INT> one1({1, 20, "1", line1});
        Literal<Literals::INT> one2({1, 22, "1", line1});
        one1.data = 1;
        one2.data = 1;
        op.left_operand = std::make_unique<ValueProducer>(std::move(one1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::SUBTRACT;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negativ leading literal") {
      auto line1 = std::make_shared<std::string>("def main() {1; -1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Literal<Literals::INT> one1({1, 13, "1", line1});
        Literal<Literals::INT> one2({1, 17, "1", line1});
        Operator op({1, 16, "-", line1});
        one1.data = 1;
        one2.data = 1;
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::NEGATIVE;
        fun.scope->nodes.push_back(std::move(one1));
        fun.scope->nodes.push_back(std::move(op));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
  }
  SECTION("Double") {
    SECTION("positive") {
      auto line1 = std::make_shared<std::string>("def main() {return .1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Literal<Literals::DOUBLE> one({1, 20, ".1", line1});
        one.data = .1;
        ret.output = std::make_unique<ValueProducer>(std::move(one));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negative") {
      auto line1 = std::make_shared<std::string>("def main() {return -.1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 20, "-", line1});
        Literal<Literals::DOUBLE> one({1, 21, ".1", line1});
        one.data = .1;
        op.right_operand = std::make_unique<ValueProducer>(std::move(one));
        op.operation = Operation::NEGATIVE;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negative operator") {
      auto line1 =
          std::make_shared<std::string>("def main() {return .1 - .1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 23, "-", line1});
        Literal<Literals::DOUBLE> one1({1, 20, ".1", line1});
        Literal<Literals::DOUBLE> one2({1, 25, ".1", line1});
        one1.data = .1;
        one2.data = .1;
        op.left_operand = std::make_unique<ValueProducer>(std::move(one1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::SUBTRACT;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negative operator nospace") {
      auto line1 = std::make_shared<std::string>("def main() {return .1-.1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Return ret({1, 13, "return", line1});
        Operator op({1, 22, "-", line1});
        Literal<Literals::DOUBLE> one1({1, 20, ".1", line1});
        Literal<Literals::DOUBLE> one2({1, 23, ".1", line1});
        one1.data = .1;
        one2.data = .1;
        op.left_operand = std::make_unique<ValueProducer>(std::move(one1));
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::SUBTRACT;
        ret.output = std::make_unique<ValueProducer>(std::move(op));
        fun.scope->nodes.push_back(std::move(ret));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
    SECTION("negativ leading literal") {
      auto line1 = std::make_shared<std::string>("def main() {.1; -.1;}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def({1, 1, "def", line1});
        EntryFunction fun({1, 5, "main", line1});
        fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        Literal<Literals::DOUBLE> one1({1, 13, ".1", line1});
        Literal<Literals::DOUBLE> one2({1, 18, ".1", line1});
        Operator op({1, 17, "-", line1});
        one1.data = .1;
        one2.data = .1;
        op.right_operand = std::make_unique<ValueProducer>(std::move(one2));
        op.operation = Operation::NEGATIVE;
        fun.scope->nodes.push_back(std::move(one1));
        fun.scope->nodes.push_back(std::move(op));
        def.definition = std::move(fun);
        expected.nodes.push_back(std::move(def));
      }

      REQUIRE(ast == expected);
    }
  }
  // TODO this section should be in a test case for operators
  SECTION("Operator substracht one negative") {
    auto line1 = std::make_shared<std::string>("def main() {return 1--1;}");
    auto ast = parse(*line1);

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun({1, 5, "main", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      Return ret({1, 13, "return", line1});
      Operator op1({1, 21, "-", line1});
      Operator op2({1, 22, "-", line1});
      Literal<Literals::INT> one1({1, 20, "1", line1});
      Literal<Literals::INT> one2({1, 23, "1", line1});
      one1.data = 1;
      one2.data = 1;
      op2.right_operand = std::make_unique<ValueProducer>(std::move(one2));
      op2.operation = Operation::NEGATIVE;
      op1.left_operand = std::make_unique<ValueProducer>(std::move(one1));
      op1.right_operand = std::make_unique<ValueProducer>(std::move(op2));
      op1.operation = Operation::SUBTRACT;
      ret.output = std::make_unique<ValueProducer>(std::move(op1));
      fun.scope->nodes.push_back(std::move(ret));
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }
  SECTION("Operator substracht two negative") {
    auto line1 = std::make_shared<std::string>("def main() {return 1---1;}");
    auto ast = parse(*line1);

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def", line1});
      EntryFunction fun({1, 5, "main", line1});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      Return ret({1, 13, "return", line1});
      Operator op1({1, 21, "-", line1});
      Operator op2({1, 22, "-", line1});
      Operator op3({1, 23, "-", line1});
      Literal<Literals::INT> one1({1, 20, "1", line1});
      Literal<Literals::INT> one2({1, 24, "1", line1});
      one1.data = 1;
      one2.data = 1;
      op3.right_operand = std::make_unique<ValueProducer>(std::move(one2));
      op3.operation = Operation::NEGATIVE;
      op2.right_operand = std::make_unique<ValueProducer>(std::move(op3));
      op2.operation = Operation::NEGATIVE;
      op1.left_operand = std::make_unique<ValueProducer>(std::move(one1));
      op1.right_operand = std::make_unique<ValueProducer>(std::move(op2));
      op1.operation = Operation::SUBTRACT;
      ret.output = std::make_unique<ValueProducer>(std::move(op1));
      fun.scope->nodes.push_back(std::move(ret));
      def.definition = std::move(fun);
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("For") {
  SECTION("Empty header") {
    auto line1 = std::make_shared<std::string>("def main() {for(;;){}}");
    auto ast = parse(*line1);

    Scope expected({0, 0, ""});
    {
      Define def_m({1, 1, "def", line1});
      EntryFunction main({1, 5, "main", line1});
      main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
      For f({1, 13, "for", line1});
      f.scope = std::make_unique<Scope>(Token(1, 20, "{", line1));

      main.scope->nodes.push_back(std::move(f));
      def_m.definition = std::move(main);
      expected.nodes.push_back(std::move(def_m));
    }
    REQUIRE(ast == expected);
  }
  SECTION("First header place") {
    SECTION("Pre defined variable") {
      auto line1 =
          std::make_shared<std::string>("def main() {var a;for(a = 0;;){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        Define def_v({1, 13, "var", line1});
        For f({1, 19, "for", line1});
        Operator op({1, 25, "=", line1});
        Literal<Literals::INT> lit({1, 27, "0", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        def_v.definition = Variable({1, 17, "a", line1});
        op.operation = Operation::ASSIGNMENT;
        lit.data = 0;

        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 23, "a", line1}));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit));
        f.variable = std::move(op);
        f.scope = std::make_unique<Scope>(Token(1, 31, "{", line1));

        main.scope->nodes.push_back(std::move(def_v));
        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Defined variable") {
      auto line1 =
          std::make_shared<std::string>("def main() {for(var a = 0;;){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        For f({1, 13, "for", line1});
        Define def_v({1, 17, "var", line1});
        Operator op({1, 23, "=", line1});
        Literal<Literals::INT> lit({1, 25, "0", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        def_v.definition = Variable({1, 21, "a", line1});
        op.operation = Operation::ASSIGNMENT;
        lit.data = 0;

        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 21, "a", line1}));
        op.right_operand = std::make_unique<ValueProducer>(std::move(lit));
        f.define = std::move(def_v);
        f.variable = std::move(op);
        f.scope = std::make_unique<Scope>(Token(1, 29, "{", line1));

        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Function") {
      auto line1 = std::make_shared<std::string>("def main() {for(fun();;){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        For f({1, 13, "for", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        f.variable = Callable({1, 17, "fun", line1});
        f.scope = std::make_unique<Scope>(Token(1, 25, "{", line1));

        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
  }
  SECTION("Second header place") {
    SECTION("Variable") {
      auto line1 =
          std::make_shared<std::string>("def main() {var a; for(;a;){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        Define def_v({1, 13, "var", line1});
        For f({1, 20, "for", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        def_v.definition = Variable({1, 17, "a", line1});
        f.condition =
            std::make_unique<ValueProducer>(Variable({1, 25, "a", line1}));
        f.scope = std::make_unique<Scope>(Token(1, 28, "{", line1));

        main.scope->nodes.push_back(std::move(def_v));
        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Function") {
      auto line1 = std::make_shared<std::string>("def main() {for(;fun();){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        For f({1, 13, "for", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        f.condition =
            std::make_unique<ValueProducer>(Callable({1, 18, "fun", line1}));
        f.scope = std::make_unique<Scope>(Token(1, 25, "{", line1));

        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Operator") {
      auto line1 =
          std::make_shared<std::string>("def main() {for(;true == fun();){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        For f({1, 13, "for", line1});
        Operator op({1, 23, "==", line1});
        Literal<Literals::BOOL> lit({1, 18, "true", line1});
        Callable fun({1, 26, "fun", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        lit.data = true;
        op.operation = Operation::EQUAL;
        op.left_operand = std::make_unique<ValueProducer>(std::move(lit));
        op.right_operand = std::make_unique<ValueProducer>(std::move(fun));

        f.condition = std::make_unique<ValueProducer>(std::move(op));
        f.scope = std::make_unique<Scope>(Token(1, 33, "{", line1));

        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
  }
  SECTION("Third section") {
    SECTION("Variable") {
      auto line1 =
          std::make_shared<std::string>("def main() {var a; for(;;a){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        Define def_v({1, 13, "var", line1});
        For f({1, 20, "for", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        def_v.definition = Variable({1, 17, "a", line1});
        f.operation = Variable({1, 26, "a", line1});
        f.scope = std::make_unique<Scope>(Token(1, 28, "{", line1));

        main.scope->nodes.push_back(std::move(def_v));
        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Function") {
      auto line1 = std::make_shared<std::string>("def main() {for(;;fun()){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        For f({1, 13, "for", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        f.operation = Callable({1, 19, "fun", line1});
        f.scope = std::make_unique<Scope>(Token(1, 25, "{", line1));

        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
    SECTION("Operator") {
      auto line1 = std::make_shared<std::string>(
          "def main() {var a;for(;;a = a + 1){}}");
      auto ast = parse(*line1);

      Scope expected({0, 0, ""});
      {
        Define def_m({1, 1, "def", line1});
        EntryFunction main({1, 5, "main", line1});
        Define def_v({1, 13, "var", line1});
        For f({1, 19, "for", line1});
        Operator op_as({1, 27, "=", line1});
        Operator op_ad({1, 31, "+", line1});
        Literal<Literals::INT> lit({1, 33, "1", line1});

        main.scope = std::make_unique<Scope>(Token(1, 12, "{", line1));
        def_v.definition = Variable({1, 17, "a", line1});
        lit.data = 1;
        op_ad.operation = Operation::ADD;
        op_ad.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 29, "a", line1}));
        op_ad.right_operand = std::make_unique<ValueProducer>(std::move(lit));

        op_as.operation = Operation::ASSIGNMENT;
        op_as.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 25, "a", line1}));
        op_as.right_operand = std::make_unique<ValueProducer>(std::move(op_ad));

        f.operation = std::move(op_as);
        f.scope = std::make_unique<Scope>(Token(1, 35, "{", line1));

        main.scope->nodes.push_back(std::move(def_v));
        main.scope->nodes.push_back(std::move(f));
        def_m.definition = std::move(main);
        expected.nodes.push_back(std::move(def_m));
      }
      REQUIRE(ast == expected);
    }
  }
}

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

/////////////////////////////////////////
// Analyser tests
/////////////////////////////////////////
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
  REQUIRE_THROWS_AS(parse("def main(){do while(true);}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){do;}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){while(true)}"), ExceptionBase<UserE>);
}
TEST_CASE("missing condition") {
  REQUIRE_THROWS_AS(parse("def main(){if(){}}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){if(){}else{}}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){do{}while();}"), ExceptionBase<UserE>);
  REQUIRE_THROWS_AS(parse("def main(){while(){}}"), ExceptionBase<UserE>);
}
