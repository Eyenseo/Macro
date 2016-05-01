#include <Catch/catch.hpp>

// Yes this is a __bad__ hack, but for testing it is white magic!
#define private public

#include "cad/macro/interpreter/OperatorProvider.h"

#include <exception.h>

using OperatorProvider = cad::macro::interpreter::OperatorProvider;
using BiOp = OperatorProvider::BinaryOperation;
using UnOp = OperatorProvider::UnaryOperation;

class TestOperatorProvider : public cad::macro::interpreter::OperatorProvider {
public:
  TestOperatorProvider(const bool initialize = false)
      : OperatorProvider() {
    if(!initialize) {
      divide_.clear();         // Deinit everything
      multiply_.clear();       // Deinit everything
      modulo_.clear();         // Deinit everything
      add_.clear();            // Deinit everything
      subtract_.clear();       // Deinit everything
      smaller_.clear();        // Deinit everything
      smaller_equal_.clear();  // Deinit everything
      greater_.clear();        // Deinit everything
      greater_equal_.clear();  // Deinit everything
      equal_.clear();          // Deinit everything
      not_equal_.clear();      // Deinit everything
      bool_.clear();           // Deinit everything
      print_.clear();          // Deinit everything
      type_of_.clear();        // Deinit everything
      negative_.clear();       // Deinit everything
      positive_.clear();       // Deinit everything
    }
  }
};

CATCH_TRANSLATE_EXCEPTION(std::exception& e) {
  std::stringstream ss;
  exception::print_exception(e, ss);
  return ss.str();
}

TEST_CASE("Binary Operations") {
  TestOperatorProvider op;
  linb::any lhs = 1;
  linb::any rhs = 1;

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
                              return linb::any_cast<int>(lhs) ==
                                     linb::any_cast<int>(rhs);
                            })));
    REQUIRE_THROWS((op.add(
        BiOp::EQUAL, int_index, int_index, [](const linb::any& lhs,
                                              const linb::any& rhs) {  // or any
          return linb::any_cast<int>(lhs) == linb::any_cast<int>(rhs);
        })));

    REQUIRE((op.has(BiOp::EQUAL, lhs, rhs)));
    REQUIRE((op.has(BiOp::EQUAL, int_index, int_index)));

    REQUIRE_FALSE((op.has(BiOp::NOT_EQUAL, lhs, rhs)));
    REQUIRE_FALSE((op.has(BiOp::NOT_EQUAL, int_index, int_index)));
  }
}

TEST_CASE("Unary Operations") {
  TestOperatorProvider op;
  linb::any rhs = 1;

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
                              return !linb::any_cast<int>(rhs);
                            })));
    REQUIRE_THROWS((op.add(UnOp::BOOL, int_index,
                           [](const linb::any& rhs) {  // or any
                             return !linb::any_cast<int>(rhs);
                           })));

    REQUIRE((op.has(UnOp::BOOL, rhs)));
    REQUIRE((op.has(UnOp::BOOL, int_index)));

    REQUIRE_FALSE((op.has(UnOp::TYPEOF, rhs)));
    REQUIRE_FALSE((op.has(UnOp::TYPEOF, int_index)));
  }
}
