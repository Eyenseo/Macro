#include <Catch/catch.hpp>

#include "cad/macro/interpreter/Stack.h"

#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/callable/Function.h"

using Function = cad::macro::ast::callable::Function;
using Callable = cad::macro::ast::callable::Callable;

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

TEST_CASE("Variable") {
  TestStack stack;

  REQUIRE(stack.variables().empty());
  REQUIRE_FALSE(stack.has_variable("foo"));
  REQUIRE_FALSE(stack.owns_variable("foo"));

  SECTION("Add") {
    stack.add_variable("foo");

    REQUIRE(stack.variables().size() == 1);
    REQUIRE(stack.has_variable("foo"));
    REQUIRE(stack.owns_variable("foo"));
    REQUIRE_FALSE(stack.is_alias("foo"));

    SECTION("Access") {
      stack.variable("foo", [](linb::any& foo) { foo = 1; });

      stack.variable("foo", [](linb::any& foo) {
        REQUIRE(linb::any_cast<int>(foo) == 1);
      });

      SECTION("Move") {
        linb::any my_foo;
        stack.variable("foo",
                       [&my_foo](linb::any& foo) { my_foo = std::move(foo); });

        int foo = linb::any_cast<int>(my_foo);
        REQUIRE(foo == 1);
      }
    }
  }
}

TEST_CASE("Function") {
  auto stack = std::make_shared<TestStack>();
  Callable call({0, 0, "fun"});
  Function orig_fun({0, 0, "fun"});

  REQUIRE(stack->functions().empty());
  REQUIRE_FALSE(stack->has_function(call));

  SECTION("Add") {
    stack->add_function(std::cref(orig_fun));

    REQUIRE(stack->functions().size() == 1);
    REQUIRE(stack->has_function(call));

    stack->function(
        call, [&](const Function& fun, auto) { REQUIRE(orig_fun == fun); });

    SECTION("Access") {
      stack->function(call, [](const Function& fun, auto) {
        // yes yes evil and so on ... We are tester, we are evil, we are legion
        const_cast<Function*>(&fun)->token.line = 1;
      });
      stack->function(call, [](const Function& fun, auto) {
        REQUIRE(fun.token.line == 1);
      });

      SECTION("Move") {
        Function my_fun;
        stack->function(
            call, [&my_fun](const Function& fun,
                            std::shared_ptr<cad::macro::interpreter::Stack>) {
              my_fun = fun;
            });

        REQUIRE(my_fun.token.line == 1);
      }
    }
  }
}

TEST_CASE("Parent") {
  auto stack_a = std::make_shared<TestStack>();
  auto stack_b = std::make_shared<TestStack>(stack_a);
  REQUIRE(stack_a == stack_b->parent());

  SECTION("Variable") {
    SECTION("Parent has not") {
      REQUIRE_FALSE(stack_b->has_variable("foo"));
      REQUIRE_FALSE(stack_b->owns_variable("foo"));
      REQUIRE_FALSE(stack_b->is_alias("foo"));

      REQUIRE_FALSE(stack_b->has_variable("bar"));
      REQUIRE_FALSE(stack_b->owns_variable("bar"));
      REQUIRE_FALSE(stack_b->is_alias("bar"));


      SECTION("Parent has") {
        stack_a->add_variable("foo");
        stack_a->variable("foo", [](linb::any& foo) { foo = 42; });

        REQUIRE(stack_b->has_variable("foo"));
        REQUIRE_FALSE(stack_b->owns_variable("foo"));
        REQUIRE_FALSE(stack_b->is_alias("foo"));

        SECTION("Add alias") {
          stack_a->variable("foo", [&stack_b](linb::any& foo) {
            stack_b->add_alias("bar", foo);
          });
          REQUIRE(stack_b->has_variable("bar"));
          REQUIRE_FALSE(stack_b->owns_variable("bar"));
          REQUIRE(stack_b->is_alias("bar"));

          SECTION("Use alias") {
            stack_b->variable("bar", [](linb::any& bar) {
              REQUIRE(linb::any_cast<int>(bar) == 42);
            });
          }

          SECTION("Remove alias") {
            stack_b->remove_alias("bar");

            REQUIRE_FALSE(stack_b->has_variable("bar"));
            REQUIRE_FALSE(stack_b->owns_variable("bar"));
            REQUIRE_FALSE(stack_b->is_alias("bar"));
          }
        }
      }
    }
  }
}
