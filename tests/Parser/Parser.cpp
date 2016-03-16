#include <Catch/catch.hpp>

#include "cad/macro/parser/Parser.h"
#include "cad/macro/ast/Scope.h"

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

TEST_CASE("Define") {
  SECTION("EntryFunction") {
    Parser p;
    auto ast = p.parse("def main() {}");

    Scope expected({0, 0, ""});
    {
      Define def({1, 1, "def"});
      EntryFunction fun({1, 5, "main"});
      fun.scope = make_unique<Scope>(Token(1, 12, "{"));
      def.definition = Define::Definition(std::move(fun));
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
      fun.scope = make_unique<Scope>(Token(1, 11, "{"));
      def.definition = Define::Definition(std::move(fun));
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
      def.definition = Define::Definition(std::move(var));
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
      var_def.definition = Define::Definition(Variable(var));
      scope.nodes.push_back(std::move(var_def));
      fun.scope = make_unique<Scope>(scope);
      def.definition = Define::Definition(std::move(fun));
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
      var1_def.definition = Define::Definition(Variable(var1));
      scope.nodes.push_back(std::move(var1_def));
      fun.parameter.push_back(var2);
      var2_def.definition = Define::Definition(Variable(var2));
      scope.nodes.push_back(std::move(var2_def));
      fun.scope = make_unique<Scope>(scope);
      def.definition = Define::Definition(std::move(fun));
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

  SECTION("Parameter") {
    Parser p;
    auto ast = p.parse("fun(herbert);");

    Scope expected({0, 0, ""});
    {
      Executable fun({1, 1, "fun"});
      Variable var1({1, 5, "herbert"});

      fun.parameter.push_back(var1);
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

      fun.parameter.push_back(var1);
      fun.parameter.push_back(var2);
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

      fun.parameter.push_back(gun);
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

      fun.parameter.push_back(gun);
      fun.parameter.push_back(hun);
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

      ret.output = Return::Output(std::move(foo));
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

      ret.output = Return::Output(std::move(fun));
      expected.nodes.push_back(std::move(ret));
    }

    REQUIRE(ast == expected);
  }
}
