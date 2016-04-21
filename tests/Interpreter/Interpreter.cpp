#include <Catch/catch.hpp>

#include "LCommand.h"

#include "cad/macro/interpreter/Interpreter.h"
#include "cad/macro/interpreter/OperatorProvider.h"

#include <cad/core/command/CommandProvider.h>
#include <cad/core/command/argument/Arguments.h>
#include <cad/core/command/MenuAdder.h>
#include <cad/core/ApplicationSettingsProvider.h>

#include <exception.h>

using Interpreter = cad::macro::interpreter::Interpreter;
using OperatorProvider = cad::macro::interpreter::OperatorProvider;
using CommandProvider = cad::core::command::CommandProvider;
using Arguments = cad::core::command::argument::Arguments;
using ApplicationSettingsProvider = cad::core::ApplicationSettingsProvider;

class TestInterpreter : public cad::macro::interpreter::Interpreter {
public:
  TestInterpreter(std::shared_ptr<CommandProvider> command_provider,
                  std::shared_ptr<OperatorProvider> operator_provider)
      : Interpreter(std::move(command_provider), std::move(operator_provider)) {
  }
};


CATCH_TRANSLATE_EXCEPTION(std::exception& e) {
  std::stringstream ss;
  exception::print_exception(e, ss);
  return ss.str();
}

TEST_CASE("Main literal return") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  SECTION("int") {
    auto ret = in.interpret("def main(){return 1;}", Arguments());
    REQUIRE(core::any_cast<int>(ret) == 1);
  }
  SECTION("double") {
    auto ret = in.interpret("def main(){return 1.1;}", Arguments());
    REQUIRE(core::any_cast<double>(ret) == Approx(1.1).epsilon(0.01));
  }
  SECTION("bool") {
    SECTION("true") {
      auto ret = in.interpret("def main(){return true;}", Arguments());
      REQUIRE(core::any_cast<bool>(ret));
    }
    SECTION("false") {
      auto ret = in.interpret("def main(){return false;}", Arguments());
      REQUIRE_FALSE(core::any_cast<bool>(ret));
    }
  }
  SECTION("string") {
    auto ret = in.interpret("def main(){return \"herbert\";}", Arguments());
    REQUIRE(core::any_cast<std::string>(ret) == "herbert");
  }
}

TEST_CASE("Return global") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  auto ret = in.interpret("var a = 1; def main(){return a;}", Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);
}

TEST_CASE("Main arguments") {
  struct Foo {
    int a = 0;
    std::string b = "1";

    bool operator==(const Foo& other) {
      return a == other.a && b == other.b;
    }
  };

  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  Foo foo;
  Arguments args;
  args.add("foo", "Foos", foo);

  auto ret = in.interpret("def main(foo){return foo;}", args);
  REQUIRE(core::any_cast<Foo>(ret) == foo);
}

TEST_CASE("Function return") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  auto ret = in.interpret("def fun(){return 1;} def main(){return fun();}",
                          Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);
}

TEST_CASE("Function named parameter") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  auto ret = in.interpret(
      "def fun(foo){return foo;} def main(){return fun(foo:1);}", Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);
}

TEST_CASE("Return global variable from function parameter") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  Interpreter in(cp, nullptr);

  auto ret = in.interpret(
      "var a = 1; def fun(foo){return foo;} def main(){return fun(foo:a);}",
      Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);
}

TEST_CASE("CommandPrvider fall through") {
  auto asp = std::make_shared<ApplicationSettingsProvider>();
  auto cp = std::make_shared<CommandProvider>(asp, nullptr);
  Interpreter in(cp, nullptr);

  SECTION("Call") {
    cad::core::command::MenuAdder m(cp, [] {});
    m.name("fun").scope("").add<LCommand>("fun", cp,
                                          [](Arguments) { return 10; });

    auto ret = in.interpret("def main(){return fun();}", Arguments());
    REQUIRE(core::any_cast<int>(ret) == 10);
  }

  SECTION("Return") {
    Arguments args;
    args.add("foo", "int", 1);

    cad::core::command::MenuAdder m(cp, [] {});
    m.name("gun").scope("").add<LCommand>("gun", cp, [](Arguments args) {
      return *args.get<int>("foo") + 2;
    }, args);

    auto ret =
        in.interpret("var a = 40; def main(){return gun(foo:a);}", Arguments());
    REQUIRE(core::any_cast<int>(ret) == 42);
  }
}

TEST_CASE("Operator") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  auto ret = in.interpret("def main(){return 1 == 1;}", Arguments());
  REQUIRE(core::any_cast<bool>(ret));

  ret = in.interpret("def main(){return 4 / 2;}", Arguments());
  REQUIRE(core::any_cast<int>(ret) == 2);
}

TEST_CASE("If") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);


  auto ret = in.interpret(
      "def main(){if(true){return true;}else{return false;}}", Arguments());
  REQUIRE(core::any_cast<bool>(ret));

  ret = in.interpret("def main(){if(false){return true;}else{return false;}}",
                     Arguments());
  REQUIRE_FALSE(core::any_cast<bool>(ret));
}

TEST_CASE("While") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  auto ret =
      in.interpret("def main(){var i = 0; while(i < 3){ i = i + 1;} return i;}",
                   Arguments());
  REQUIRE(core::any_cast<int>(ret) == 3);


  ret = in.interpret("def main(){var i = 0; while(i < 3){ i = i + 1; if(i == "
                     "2){break;}} return i;}",
                     Arguments());
  REQUIRE(core::any_cast<int>(ret) == 2);

  ret = in.interpret(
      "def main(){var i = 0; while(i < 3){ i = i +1; return i;} return i;}",
      Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);

  ret = in.interpret(
      "def main(){var i = 0; while(i < 3){ i = i +1; return 42;} return i;}",
      Arguments());
  REQUIRE(core::any_cast<int>(ret) == 42);
}

TEST_CASE("Print") {
  std::stringstream ss;
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op, ss);

  SECTION("Single Integer") {
    auto ret = in.interpret("def main(){print 1;}", Arguments());
    REQUIRE(ret.empty());
    REQUIRE(ss.str() == "1");
  }
  SECTION("String, Integer, Double, Bool via Operator") {
    auto ret = in.interpret("def main(){print \"Herbert\" + 1 + 0.42 + true;}",
                            Arguments());
    REQUIRE(ret.empty());
    REQUIRE(ss.str() == "Herbert10.42true");
  }
}

TEST_CASE("Typeof") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  SECTION("Integer") {
    auto ret =
        in.interpret("def main(){return typeof 1 == \"int\";}", Arguments());
    REQUIRE(core::any_cast<bool>(ret));
  }
  SECTION("String") {
    auto ret = in.interpret("def main(){return typeof \"\" == \"string\";}",
                            Arguments());
    REQUIRE(core::any_cast<bool>(ret));
  }
  SECTION("Var") {
    auto ret =
        in.interpret("def main(){var a = true; return typeof a != \"string\";}",
                     Arguments());
    REQUIRE(core::any_cast<bool>(ret));
  }
}

TEST_CASE("DoWhile") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  auto ret = in.interpret(
      "def main(){var i = 0; do{ i = i +1;}while(i < 3); return i;}",
      Arguments());
  REQUIRE(core::any_cast<int>(ret) == 3);
}

TEST_CASE("Missing Function") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  using EXC_TAIL = Exc<Interpreter::E, Interpreter::E::TAIL>;  // Catch issue
  REQUIRE_THROWS_AS(in.interpret("def main(){fun();}", Arguments()), EXC_TAIL);
}

TEST_CASE("Missing Operator") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  using EXC_TAIL = Exc<Interpreter::E, Interpreter::E::TAIL>;  // Catch issue
  REQUIRE_THROWS_AS(in.interpret("def main(){\"foo\" - \"bar\";}", Arguments()),
                    EXC_TAIL);
}

TEST_CASE("Assign in Condition") {
  auto cp = std::make_shared<CommandProvider>(nullptr, nullptr);
  auto op = std::make_shared<OperatorProvider>();
  Interpreter in(cp, op);

  auto ret = in.interpret(
      "def main(){var i = 0; if(i = 1){ return 1;} else {return 0;}}",
      Arguments());
  REQUIRE(core::any_cast<int>(ret) == 1);
}

// FIXME test history stack  implementation
