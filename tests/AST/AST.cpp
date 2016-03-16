#include <Catch/catch.hpp>

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Return.h"
#include "cad/macro/ast/executable/Function.h"
#include "cad/macro/ast/executable/EntryFunction.h"

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::executable;

namespace {
// https://isocpp.org/files/papers/N3656.txt
template <class T>
struct UniqueIf {
  typedef std::unique_ptr<T> SingleObject;
};

template <class T>
struct UniqueIf<T[]> {
  typedef std::unique_ptr<T[]> UnknownBound;
};

template <class T, size_t N>
struct UniqueIf<T[N]> {
  typedef void KnownBound;
};

template <class T, class... Args>
typename UniqueIf<T>::SingleObject make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename UniqueIf<T>::UnknownBound make_unique(size_t n) {
  typedef typename std::remove_extent<T>::type U;
  return std::unique_ptr<T>(new U[n]());
}

template <class T, class... Args>
typename UniqueIf<T>::KnownBound make_unique(Args&&...) = delete;
}

TEST_CASE("AST Comparison") {
  SECTION("Empty") {
    AST a({0, 0, ""});
    AST b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Line different") {
    AST a({0, 0, ""});
    AST b({1, 0, ""});
    REQUIRE_FALSE(a == b);
  }
  SECTION("Column different") {
    AST a({0, 0, ""});
    AST b({0, 2, ""});
    REQUIRE_FALSE(a == b);
  }
  SECTION("Token different") {
    AST a({0, 0, ""});
    AST b({0, 0, "Foo"});
    REQUIRE_FALSE(a == b);
  }
  SECTION("All different") {
    AST a({0, 0, ""});
    AST b({1, 1, "Foo"});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Scope Comparison") {
  SECTION("Empty") {
    Scope a({0, 0, ""});
    Scope b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    b.nodes.emplace_back(Scope({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Different Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    b.nodes.emplace_back(Scope({1, 0, ""}));
    REQUIRE_FALSE(a == b);
  }
  SECTION("Scope/No-Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Executable Comparison") {
  SECTION("Parameterless") {
    Executable a({0, 0, ""});
    Executable b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Parameter") {
    Executable a({0, 0, ""});
    a.parameter.emplace_back(Variable({0, 0, ""}));
    Executable b({0, 0, ""});
    b.parameter.emplace_back(Variable({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Parameter/Parameterless") {
    Executable a({0, 0, ""});
    a.parameter.emplace_back(Variable({0, 0, ""}));
    Executable b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Function Comparison") {
  SECTION("No-Scope") {
    Function a({0, 0, ""});
    Function b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Scope") {
    Function a({0, 0, ""});
    a.scope = make_unique<Scope>(Token(0, 0, ""));
    Function b({0, 0, ""});
    b.scope = make_unique<Scope>(Token(0, 0, ""));
    REQUIRE(a == b);
  }
  SECTION("Scope/No-Scope") {
    Function a({0, 0, ""});
    a.scope = make_unique<Scope>(Token(0, 0, ""));
    Function b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Define Comparison") {
  SECTION("No-Definition") {
    Define a({0, 0, ""});
    Define b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Definition") {
    Define a({0, 0, ""});
    a.definition.emplace(Function({0, 0, ""}));
    Define b({0, 0, ""});
    b.definition.emplace(Function({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Definition/No-Definition") {
    Define a({0, 0, ""});
    a.definition.emplace(Function({0, 0, ""}));
    Define b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Return Comparison") {
  SECTION("No-Definition") {
    Return a({0, 0, ""});
    Return b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    b.output.emplace(Variable({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Different Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    b.output.emplace(Executable({0, 0, ""}));
    REQUIRE_FALSE(a == b);
  }
  SECTION("Definition/No-Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}
