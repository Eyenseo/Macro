#include <Catch/catch.hpp>

#include "cad/macro/parser/Parser.h"
#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/ValueProducer.h"

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::executable;
using namespace cad::macro::ast::logic;
using namespace cad::macro::ast::loop;

TEST_CASE("Define") {
  SECTION("EntryFunction") {
    Parser p;
    auto ast = p.parse("def main() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def"});
      EntryFunction fun({1, 5, "main"});
      fun.scope = std::make_unique<Scope>(Token(1, 12, "{"));
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    Parser p;
    auto ast = p.parse("def fun() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def"});
      Function fun({1, 5, "fun"});
      fun.scope = std::make_unique<Scope>(Token(1, 11, "{"));
      def.definition.emplace(std::move(fun));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Variable") {
    Parser p;
    auto ast = p.parse("var foo;");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "var"});
      Variable var({1, 5, "foo"});
      def.definition.emplace(std::move(var));
      expected.nodes.push_back(std::move(def));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Parameter") {
    Parser p;
    auto ast = p.parse("def fun(herbert) {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def"});
      Function fun({1, 5, "fun"});
      Variable var({1, 9, "herbert"});
      Scope scope({1, 18, "{"});
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

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def"});
      Function fun({1, 5, "fun"});
      Variable var1({1, 9, "herbert"});
      Variable var2({1, 18, "berta"});
      Scope scope({1, 25, "{"});
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

TEST_CASE("Executable") {
  SECTION("Parameterless") {
    Parser p;
    auto ast = p.parse("fun();");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});

      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Space after function name") {
    Parser p;
    //TODO check message?
    REQUIRE_THROWS(p.parse("fun ();"));
  }

  SECTION("Parameter") {
    Parser p;
    auto ast = p.parse("fun(herbert);");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});
      Variable var1({1, 5, "herbert"});

      fun.parameter.emplace_back(var1);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Multiple Parameter") {
    Parser p;
    auto ast = p.parse("fun(herbert, berta);");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});
      Variable var1({1, 5, "herbert"});
      Variable var2({1, 14, "berta"});

      fun.parameter.emplace_back(var1);
      fun.parameter.emplace_back(var2);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function Parameter") {
    Parser p;
    auto ast = p.parse("fun(gun());");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});
      Executable gun({1, 5, "gun"});

      fun.parameter.emplace_back(gun);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Multiple Function Parameter") {
    Parser p;
    auto ast = p.parse("fun(gun(), hun());");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});
      Executable gun({1, 5, "gun"});
      Executable hun({1, 12, "hun"});

      fun.parameter.emplace_back(gun);
      fun.parameter.emplace_back(hun);
      expected.nodes.push_back(std::move(fun));
    }

    REQUIRE(ast == expected);
  }
}

TEST_CASE("Return") {
  SECTION("Variable") {
    Parser p;
    auto ast = p.parse("return foo;");

    Scope expected({0, 0, ""});
    {
      Return ret({1, 1, "return"});
      Variable foo({1, 8, "foo"});

      ret.output = std::make_unique<ValueProducer>(std::move(foo));
      expected.nodes.push_back(std::move(ret));
    }

    REQUIRE(ast == expected);
  }

  SECTION("Function") {
    Parser p;
    auto ast = p.parse("return fun();");

    Scope expected({0, 0, ""});
    {
      Return ret({1, 1, "return"});
      Executable fun({1, 8, "fun"});

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

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        iff.condition = std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 6, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Function") {
      Parser p;
      auto ast = p.parse("if(fun()){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        iff.condition =
            std::make_unique<ValueProducer>(Executable({1, 4, "fun"}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
    SECTION("Literal") {
      Parser p;
      auto ast = p.parse("if(true){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        Literal<Literals::BOOL> lit({1, 4, "true"});
        lit.data = true;
        iff.condition = std::make_unique<ValueProducer>(lit);
        iff.true_scope = std::make_unique<Scope>(Token(1, 9, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Operator") {
    SECTION("Equal") {
      Parser p;
      auto ast = p.parse("if(a == b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, "=="});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not Equal") {
      Parser p;
      auto ast = p.parse("if(a != b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, "!="});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        op.operation = BinaryOperation::NOT_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater") {
      Parser p;
      auto ast = p.parse("if(a > b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, ">"});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 8, "b"}));
        op.operation = BinaryOperation::GREATER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Greater Equal") {
      Parser p;
      auto ast = p.parse("if(a >= b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, ">="});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        op.operation = BinaryOperation::GREATER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller") {
      Parser p;
      auto ast = p.parse("if(a < b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, "<"});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 8, "b"}));
        op.operation = BinaryOperation::SMALLER;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 10, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Smaller Equal") {
      Parser p;
      auto ast = p.parse("if(a <= b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, "<="});
        op.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        op.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        op.operation = BinaryOperation::SMALLER_EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not var") {
      Parser p;
      auto ast = p.parse("if(!a){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        UnaryOperator op({1, 4, "!"});
        op.operand = std::make_unique<ValueProducer>(Variable({1, 5, "a"}));
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 7, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Not operator") {
      Parser p;
      auto ast = p.parse("if(!(a == b)){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op2({1, 8, "=="});
        op2.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 6, "a"}));
        op2.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 11, "b"}));
        op2.operation = BinaryOperation::EQUAL;
        UnaryOperator op({1, 4, "!"});
        op.operand = std::make_unique<ValueProducer>(op2);
        op.operation = UnaryOperation::NOT;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 14, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Literals") {
    SECTION("Boolean") {
      Parser p;
      auto ast = p.parse("if(true == false){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 9, "=="});
        Literal<Literals::BOOL> left({1, 4, "true"});
        Literal<Literals::BOOL> right({1, 12, "false"});
        left.data = true;
        right.data = false;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 18, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Integer") {
      Parser p;
      auto ast = p.parse("if(1 == 1){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 6, "=="});
        Literal<Literals::INT> left({1, 4, "1"});
        Literal<Literals::INT> right({1, 9, "1"});
        left.data = 1;
        right.data = 1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 11, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Double") {
      Parser p;
      auto ast = p.parse("if(.1 == .1){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 7, "=="});
        Literal<Literals::DOUBLE> left({1, 4, ".1"});
        Literal<Literals::DOUBLE> right({1, 10, ".1"});
        left.data = 0.1;
        right.data = 0.1;
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("String") {
      Parser p;
      // The escaped " counts one!
      auto ast = p.parse("if(\"a\" == \"a\"){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator op({1, 7, "=="});
        Literal<Literals::STRING> left({1, 6, "\"a\""});
        Literal<Literals::STRING> right({1, 12, "\"a\""});
        left.data = "a";
        right.data = "a";
        op.left_operand = std::make_unique<ValueProducer>(left);
        op.right_operand = std::make_unique<ValueProducer>(right);
        op.operation = BinaryOperation::EQUAL;

        iff.condition = std::make_unique<ValueProducer>(op);
        iff.true_scope = std::make_unique<Scope>(Token(1, 13, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }

  SECTION("Combination") {
    SECTION("Brackets") {
      Parser p;
      auto ast = p.parse("if(a == b && (a == c || c == b)){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator a_eq_c({1, 17, "=="});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 15, "a"}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 20, "c"}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator c_eq_b({1, 27, "=="});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 25, "c"}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 30, "b"}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 22, "||"});
        op_or.left_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        BinaryOperator a_eq_b({1, 6, "=="});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        a_eq_b.operation = BinaryOperation::EQUAL;

        BinaryOperator op_and({1, 11, "&&"});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(op_or);
        op_and.operation = BinaryOperation::AND;

        iff.condition = std::make_unique<ValueProducer>(op_and);
        iff.true_scope = std::make_unique<Scope>(Token(1, 33, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Bracketless") {
      Parser p;
      auto ast = p.parse("if(a == b && a == c || c == b){}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        BinaryOperator a_eq_b({1, 6, "=="});
        a_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        a_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 9, "b"}));
        a_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator a_eq_c({1, 16, "=="});
        a_eq_c.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 14, "a"}));
        a_eq_c.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 19, "c"}));
        a_eq_c.operation = BinaryOperation::EQUAL;
        BinaryOperator op_and({1, 11, "&&"});
        op_and.left_operand = std::make_unique<ValueProducer>(a_eq_b);
        op_and.right_operand = std::make_unique<ValueProducer>(a_eq_c);
        op_and.operation = BinaryOperation::AND;

        BinaryOperator c_eq_b({1, 26, "=="});
        c_eq_b.left_operand =
            std::make_unique<ValueProducer>(Variable({1, 24, "c"}));
        c_eq_b.right_operand =
            std::make_unique<ValueProducer>(Variable({1, 29, "b"}));
        c_eq_b.operation = BinaryOperation::EQUAL;
        BinaryOperator op_or({1, 21, "||"});
        op_or.left_operand = std::make_unique<ValueProducer>(op_and);
        op_or.right_operand = std::make_unique<ValueProducer>(c_eq_b);
        op_or.operation = BinaryOperation::OR;

        iff.condition = std::make_unique<ValueProducer>(op_or);
        iff.true_scope = std::make_unique<Scope>(Token(1, 31, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }

    SECTION("Else") {
      Parser p;
      auto ast = p.parse("if(a){}else{}");

      Scope expected({0, 0, ""});
      {
        If iff({1, 1, "if"});
        iff.condition = std::make_unique<ValueProducer>(Variable({1, 4, "a"}));
        iff.true_scope = std::make_unique<Scope>(Token(1, 6, "{"));
        iff.false_scope = std::make_unique<Scope>(Token(1, 12, "{"));
        expected.nodes.push_back(std::move(iff));
      }

      REQUIRE(ast == expected);
    }
  }
}

TEST_CASE("While") {  // Basically an if
  Parser p;
  auto ast = p.parse("while(a){}");

  Scope expected({0, 0, ""});
  {
    While w({1, 1, "while"});
    w.condition = std::make_unique<ValueProducer>(Variable({1, 7, "a"}));
    w.scope = std::make_unique<Scope>(Token(1, 9, "{"));
    expected.nodes.push_back(std::move(w));
  }

  REQUIRE(ast == expected);
}

TEST_CASE("\"free\" operators") {
}

// FIXME
// TEST_CASE("For") {
//   Parser p;
//   auto ast = p.parse("for(var a = 0; a < 10; a = a + 1){}");

//   Scope expected({0, 0, ""});
//   {
//     For f({1, 1, "For"});
//     f.condition = std::make_unique<ValueProducer>(Variable({1, 4, "a"}));


//     f.true_scope = std::make_unique<Scope>(Token(1, 6, "{"));
//     f.false_scope = std::make_unique<Scope>(Token(1, 12, "{"));
//     expected.nodes.push_back(std::move(f));
//   }

//   REQUIRE(ast == expected);
// }


TEST_CASE("Complete") {
  const std::string raw_macro = "\n"
                                "var a = true;            \n"
                                "var b = 2;               \n"
                                "var c = \" 3\";          \n"
                                "                         \n"
                                "                         \n"
                                "def fun(foo) {           \n"
                                "  var bar;               \n"
                                "                         \n"
                                "  if(foo == a) {         \n"
                                "    bar = foo;           \n"
                                "  } else {               \n"
                                "    bar = b;             \n"
                                "  }                      \n"
                                "                         \n"
                                "  return bar;            \n"
                                "}                        \n"
                                "                         \n"
                                "                         \n"
                                "def main(foo, bar) {     \n"
                                "  var baz = foo;         \n"
                                "                         \n"
                                "  fun(baz);              \n"
                                "}                        \n"
                                "                         \n";
  Parser p;
  WARN(p.parse(raw_macro));
}
