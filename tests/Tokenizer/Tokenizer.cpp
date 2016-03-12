#include <Catch/catch.hpp>

#include "cad/macro/parser/Tokenizer.h"
#include "cad/macro/parser/Token.h"

using namespace cad::macro::parser;
TEST_CASE("String Token", "[Tokenizer]") {
  std::vector<std::string> expected = {
      "var",    "a",    "=",   "true",   ";",                   // Line  2
      "var",    "b",    "=",   "2",      ";",                   // Line  3
      "var",    "c",    "=",   "\" 3\"", ";",                   // Line  4
      "def",    "fun",  "(",   "foo",    ")", "{",              // Line  7
      "var",    "bar",  ";",                                    // Line  8
      "if",     "(",    "foo", "==",     "a", ")",   "{",       // Line 10
      "bar",    "=",    "foo", ";",                             // Line 11
      "}",      "else", "{",                                    // Line 12
      "bar",    "=",    "b",   ";",                             // Line 13
      "}",                                                      // Line 14
      "return", "bar",  ";",                                    // Line 16
      "}",                                                      // Line 17
      "def",    "main", "(",   "foo",    ",", "bar", ")", "{",  // Line 20
      "var",    "baz",  "=",   "foo",    ";",                   // Line 21
      "fun",    "(",    "baz", ")",      ";",                   // Line 23
      "}"                                                       // Line 24
  };

  SECTION("well fromated") {
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

    auto tokens = tokenizer::tokenize(raw_macro);

    std::vector<std::string> stokens;
    stokens.reserve(tokens.size());
    for(const auto& t : tokens) {
      stokens.push_back(t.token);
    }

    REQUIRE(stokens == expected);
  }

  SECTION("compact formated") {
    const std::string raw_macro = "var a=true;"
                                  "var b=2;"
                                  "var c=\" 3\";"
                                  "def fun(foo){"
                                  "var bar;"
                                  "if(foo==a){"
                                  "bar=foo;"
                                  "}else{"
                                  "bar=b;"
                                  "}"
                                  "return bar;"
                                  "}"
                                  "def main(foo,bar) {"
                                  "var baz=foo;"
                                  "fun(baz);"
                                  "}";

    auto tokens = tokenizer::tokenize(raw_macro);

    std::vector<std::string> stokens;
    stokens.reserve(tokens.size());
    for(const auto& t : tokens) {
      stokens.push_back(t.token);
    }

    REQUIRE(stokens == expected);
  }
};
