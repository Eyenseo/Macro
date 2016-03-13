#include <Catch/catch.hpp>

#include "cad/macro/parser/Tokenizer.h"
#include "cad/macro/parser/Token.h"

using namespace cad::macro::parser;

std::vector<std::string> tokens_to_strings(const std::vector<Token>& tokens) {
  std::vector<std::string> stokens;
  stokens.reserve(tokens.size());

  for(const auto& t : tokens) {
    stokens.push_back(t.token);
  }
  return stokens;
}

TEST_CASE("Syntax Free") {
  SECTION("Valid Syntax") {
    std::vector<std::string> expected = {"var", "a", ";"};
    const std::string raw_macro = "var a;";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("Invalid Syntax") {
    std::vector<std::string> expected = {"var", "a"};
    const std::string raw_macro = "var a";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
}

TEST_CASE("Special Tokens") {
  SECTION("=") {
    std::vector<std::string> expected = {"="};
    const std::string raw_macro = "=";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("==") {
    std::vector<std::string> expected = {"=="};
    const std::string raw_macro = "==";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("===") {
    std::vector<std::string> expected = {"==", "="};
    const std::string raw_macro = "===";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("(") {
    std::vector<std::string> expected = {"("};
    const std::string raw_macro = "(";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("()") {
    std::vector<std::string> expected = {"(", ")"};
    const std::string raw_macro = "()";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("(,)") {
    std::vector<std::string> expected = {"(", ",", ")"};
    const std::string raw_macro = "(,)";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
}

TEST_CASE("String Tokens") {
  SECTION("Escape") {
    std::vector<std::string> expected = {"\"Herbert is a \\\" nice guy\""};
    const std::string raw_macro = "\"Herbert is a \\\" nice guy\"";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("Escapex2") {
    std::vector<std::string> expected = {"\"Herbert is a \\\\\"", "nice", "guy",
                                         "\""};
    const std::string raw_macro = "\"Herbert is a \\\\\" nice guy\"";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("Escapex3") {
    std::vector<std::string> expected = {"\"Herbert is a \\\\\\\" nice guy\""};
    const std::string raw_macro = "\"Herbert is a \\\\\\\" nice guy\"";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
}

TEST_CASE("Float Tokens") {
  SECTION("Normal Float Tokens") {
    std::vector<std::string> expected = {"4.2"};
    const std::string raw_macro = "4.2";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("Short Float Tokens") {
    std::vector<std::string> expected = {".2"};
    const std::string raw_macro = ".2";
    auto tokens = tokenizer::tokenize(raw_macro);

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
}

TEST_CASE("Format") {
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

  SECTION("Well Fromated") {
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

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
  SECTION("Compact Formated") {
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

    REQUIRE(tokens_to_strings(tokens) == expected);
  }
}

TEST_CASE("Token Info") {
  std::vector<Token> expected = {{1, 1, "1"}, {2, 3, "2"}, {4, 2, "4"}};
  const std::string raw_macro = "1\n   2\n\n\t 4";
  auto tokens = tokenizer::tokenize(raw_macro);

  REQUIRE(tokens == expected);
}
