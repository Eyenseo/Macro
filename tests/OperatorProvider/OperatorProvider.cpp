#include <Catch/catch.hpp>

#include "cad/macro/interpreter/OperatorProvider.h"

#include <exception.h>

using OperatorProvider = cad::macro::interpreter::OperatorProvider;
using BiOp = OperatorProvider::BinaryOperation;
using UnOp = OperatorProvider::UnaryOperation;

class TestOperatorProvider : public cad::macro::interpreter::OperatorProvider {
public:
  TestOperatorProvider()
      : OperatorProvider(false) {
  }
};

CATCH_TRANSLATE_EXCEPTION(std::exception& e) {
  std::stringstream ss;
  exception::print_exception(e, ss);
  return ss.str();
}

TEST_CASE("Binary Operations") {
  TestOperatorProvider op;
  ::core::any lhs = 1;
  ::core::any rhs = 1;

  SECTION("Template") {
    REQUIRE_FALSE((op.has<int, int, BiOp::EQUAL, BiOp::DIVIDE>()));
    REQUIRE_FALSE((op.has<BiOp::EQUAL, BiOp::DIVIDE>(lhs, rhs)));

    REQUIRE_NOTHROW((op.add<int, int, BiOp::EQUAL, BiOp::DIVIDE>()));
    REQUIRE_THROWS((op.add<int, int, BiOp::EQUAL, BiOp::DIVIDE>()));

    REQUIRE((op.has<int, int, BiOp::EQUAL, BiOp::DIVIDE>()));
    REQUIRE((op.has<BiOp::EQUAL, BiOp::DIVIDE>(lhs, rhs)));

    REQUIRE_FALSE((op.has<int, int, BiOp::NOT_EQUAL>()));
    REQUIRE_FALSE((op.has<BiOp::NOT_EQUAL>(lhs, rhs)));
  }

  SECTION("Manuel") {
    auto int_index = std::type_index(typeid(int));

    REQUIRE_FALSE((op.has(BiOp::EQUAL, lhs, rhs)));
    REQUIRE_FALSE((op.has(BiOp::EQUAL, int_index, int_index)));

    REQUIRE_NOTHROW((op.add(BiOp::EQUAL, int_index, int_index,
                            [](auto& lhs, auto& rhs) {  // either auto&
                              return ::core::any_cast<int>(lhs) ==
                                     ::core::any_cast<int>(rhs);
                            })));
    REQUIRE_THROWS((op.add(
        BiOp::EQUAL, int_index, int_index,
        [](const ::core::any& lhs, const ::core::any& rhs) {  // or any
          return ::core::any_cast<int>(lhs) == ::core::any_cast<int>(rhs);
        })));

    REQUIRE((op.has(BiOp::EQUAL, lhs, rhs)));
    REQUIRE((op.has(BiOp::EQUAL, int_index, int_index)));

    REQUIRE_FALSE((op.has(BiOp::NOT_EQUAL, lhs, rhs)));
    REQUIRE_FALSE((op.has(BiOp::NOT_EQUAL, int_index, int_index)));
  }
}

TEST_CASE("Unary Operations") {
  TestOperatorProvider op;
  ::core::any rhs = 1;

  SECTION("Template") {
    REQUIRE_FALSE((op.has<int, UnOp::BOOL>()));
    REQUIRE_FALSE((op.has<UnOp::BOOL>(rhs)));

    REQUIRE_NOTHROW((op.add<int, UnOp::BOOL>()));
    REQUIRE_THROWS((op.add<int, UnOp::BOOL>()));

    REQUIRE((op.has<int, UnOp::BOOL>()));
    REQUIRE((op.has<UnOp::BOOL>(rhs)));

    REQUIRE_FALSE((op.has<int, UnOp::TYPEOF>()));
    REQUIRE_FALSE((op.has<UnOp::TYPEOF>(rhs)));
  }

  SECTION("Manuel") {
    auto int_index = std::type_index(typeid(int));

    REQUIRE_FALSE((op.has(UnOp::BOOL, rhs)));
    REQUIRE_FALSE((op.has(UnOp::BOOL, int_index)));

    REQUIRE_NOTHROW((op.add(UnOp::BOOL, int_index,
                            [](auto& rhs) {  // either auto&
                              return !::core::any_cast<int>(rhs);
                            })));
    REQUIRE_THROWS((op.add(UnOp::BOOL, int_index,
                           [](const ::core::any& rhs) {  // or any
                             return !::core::any_cast<int>(rhs);
                           })));

    REQUIRE((op.has(UnOp::BOOL, rhs)));
    REQUIRE((op.has(UnOp::BOOL, int_index)));

    REQUIRE_FALSE((op.has(UnOp::TYPEOF, rhs)));
    REQUIRE_FALSE((op.has(UnOp::TYPEOF, int_index)));
  }
}
