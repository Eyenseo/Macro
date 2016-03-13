#include <Catch/catch.hpp>

#include "cad/macro/parser/Parser.h"
#include "cad/macro/ast/Scope.h"

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::executable;

TEST_CASE("EntryPoint") {
  Parser p;
  auto ast = p.parse("def main() {}");

  Scope expected(Token(0, 0, ""));
  {
    Define def(Token(1, 1, "def"));
    EntryFunction fun(Token(1, 5, "main"));
    fun.set_scope(Scope(Token(1, 12, "{")));
    def.define(std::move(fun));
    expected.append(std::move(def));
  }

  REQUIRE(ast == expected);
}
