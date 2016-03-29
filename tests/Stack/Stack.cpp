#include <Catch/catch.hpp>

#include "cad/macro/interpreter/Stack.h"

#include "cad/macro/ast/callable/Function.h"

using Function = cad::macro::ast::callable::Function;

class TestStack : public cad::macro::interpreter::Stack {
public:
  TestStack() {
  }
  TestStack(std::shared_ptr<TestStack> parent)
      : cad::macro::interpreter::Stack(
            std::dynamic_pointer_cast<Stack>(parent)) {
  }
  auto& variables() {
    return variables_;
  }
  auto& functions() {
    return functions_;
  }
};


CATCH_TRANSLATE_EXCEPTION(std::exception& e) {
  std::stringstream ss;
  exception::print_exception(e, ss);
  return ss.str();
}

TEST_CASE("Parent") {
  auto stack_a = std::make_shared<TestStack>();
  auto stack_b = std::make_shared<TestStack>(stack_a);
  REQUIRE(stack_a == stack_b->parent());
}

TEST_CASE("Variable") {
  TestStack stack;

  REQUIRE(stack.variables().empty());

  SECTION("Add") {
    stack.add_variable("foo");

    REQUIRE(stack.variables().size() == 1);

    SECTION("Access") {
      stack.variable("foo", [](core::any& foo) { foo = 1; });

      stack.variable("foo", [](core::any& foo) {
        REQUIRE(core::any_cast<int>(foo) == 1);
      });

      SECTION("Move") {
        core::any my_foo;
        stack.variable("foo",
                       [&my_foo](core::any& foo) { my_foo = std::move(foo); });

        int foo = core::any_cast<int>(my_foo);
        REQUIRE(foo == 1);
      }
    }
  }
}

TEST_CASE("Function") {
  TestStack stack;
  Function orig_fun({0, 0, ""});

  REQUIRE(stack.functions().empty());

  SECTION("Add") {
    stack.add_function("fun", std::ref(orig_fun));

    REQUIRE(stack.functions().size() == 1);
    stack.function("fun", [&](Function& fun) { REQUIRE(orig_fun == fun); });

    SECTION("Access") {
      stack.function("fun", [](Function& fun) { fun.token.line = 1; });

      stack.function("fun",
                     [](Function& fun) { REQUIRE(fun.token.line == 1); });

      SECTION("Move") {
        Function my_fun;
        stack.function("fun",
                       [&my_fun](Function& fun) { my_fun = std::move(fun); });

        REQUIRE(my_fun != orig_fun);
        REQUIRE(my_fun.token.line == 1);
      }
    }
  }
}
