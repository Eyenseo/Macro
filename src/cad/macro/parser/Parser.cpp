#include "cad/macro/parser/Parser.h"

#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/parser/Analyser.h"
#include "cad/macro/parser/Message.h"
#include "cad/macro/parser/Tokenizer.h"

#include <exception.h>

#include <cassert>
#include <experimental/optional>
#include <regex>
#include <string>

namespace cad {
namespace macro {
namespace parser {
namespace {
using ConversionExc = Exc<InternalE, InternalE::BAD_CONVERSION>;
using OperatorExc = Exc<InternalE, InternalE::MISSING_OPERATOR>;
using UserSourceExc = Exc<UserE, UserE::SOURCE>;
using UserTailExc = Exc<UserE, UserE::TAIL>;
using UserExc = ExceptionBase<UserE>;

template <typename T>
using is_Function = typename std::enable_if<
    std::is_same<T, ast::callable::Function>::value ||
        std::is_same<T, ast::callable::EntryFunction>::value,
    bool>;
template <typename T>
using is_Operator =
    typename std::enable_if<std::is_same<T, ast::Operator>::value, bool>;

/**
 * @brief  Helper struct that keeps track of the Tokens and the file that is
 *         being parsed
 */
struct Tokens {
  const std::vector<Token>& tokens;
  const std::string file;

  const Token& at(size_t i) const {
    return tokens.at(i);
  }

  size_t size() const {
    return tokens.size();
  }
};

static std::vector<std::string> keywords{
    "if",   "else",  "do",     "while", "for",   "var",    "def",  "continue",
    "main", "break", "return", "true",  "false", "typeof", "print"};

//////////////////////////////////////////
/// Exception
//////////////////////////////////////////
/**
 * @brief  Converts a Node to a Token
 *
 * @param  node  The node
 *
 * @return Token of the Node
 */
Token node_to_token(const ast::Scope::Node& node);
/**
 * @brief  Adds info to an Exception
 *
 * @param  token           The token that is being reported
 * @param  file            The file name / macro name
 * @param  e               The Exception that information is added to
 * @param  fun             The function to be executed after file:line:column
 * @param  arrow_position  The arrow position - normally under the given token
 */
template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun, const size_t arrow_position);
/**
 * @brief  Adds info to an Exception the arrow will be at the begin of the given
 *         token
 *
 * @param  token  The token that is being reported
 * @param  file   The file name / macro name
 * @param  e      The Exception that information is added to
 * @param  fun    The function to be executed after file:line:column
 */
template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun);
/**
 * @brief  Adds info to an Exception the arrow will be at the end of the given
 *         token
 *
 * @param  token  The token that is being reported
 * @param  file   The file name / macro name
 * @param  e      The Exception that information is added to
 * @param  fun    The function to be executed after file:line:column
 */
template <typename FUN>
void add_exception_info_end(const Token& token, const std::string& file,
                            UserExc& e, FUN fun);
/**
 * @brief  Adds info to an Exception the arrow will be at the begin of the given
 *         token
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  e       The Exception that information is added to
 * @param  fun     The function to be executed after file:line:column
 */
template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token, UserExc& e,
                        FUN fun);
/**
 * @brief  Throws an Exception with the reason of an unexpected Token
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @throws UserSourceExc
 */
[[noreturn]] void throw_unexprected_token(const Tokens& tokens,
                                          const size_t token);
/**
 * @brief  Throws an Exception with the reason of an unexpected Token
 *
 * @param  token  The token that is being reported
 * @param  file   The file name / macro name
 *
 * @throws UserSourceExc
 */
[[noreturn]] void throw_unexprected_token(const Token& token,
                                          const std::string& file);
/**
 * @brief  Throws an Exception with the reason of an conversation error
 *
 * @param  file    The file name / macro name
 * @param  line    The line
 * @param  prefix  The prefix - describing what was not convertible to a
 *                 ValueProducer
 *
 * @throws ConversionExc
 */
[[noreturn]] void throw_conversion(const char* const file, const size_t line,
                                   const char* const prefix);
/**
 * @brief  Checks that no extra space is in between the brackets and the
 *         function identifier
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 *
 * @throws UserSourceExc
 */
void expect_no_space_between_bracket(const Tokens& tokens, const size_t token);

//////////////////////////////////////////
/// Token reading
//////////////////////////////////////////
/**
 * @brief  Expects to find the given Token, else it throws
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 * @param  token_literal  The token literal that is expected
 *
 * @throws UserSourceExc
 */
void expect_token(const Tokens& tokens, size_t& token,
                  const char* const token_literal);
/**
 * @brief  Expects to find the given Token
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 * @param  token_literal  The token literal that is expected
 *
 * @returns true if the expected token was found, else false
 */
bool read_token(const Tokens& tokens, size_t& token,
                const char* const token_literal);
/**
 * @brief  Expects to find the given Token, else it throws
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 * @param  token_regex  The regex that matches the token that is expected
 *
 * @return true if the regex had a match, else false
 */
bool read_token(const Tokens& tokens, size_t& token,
                const std::regex& token_regex);

//////////////////////////////////////////
/// Literal parsing
//////////////////////////////////////////
/**
 * @brief  Unescapes the parsed string Literal
 */
void unescape_string(std::string& s);
/**
 * @brief  Tries to parse the current token as a bool literal
 *
 * @return The optional parsed bool literal
 */
std::experimental::optional<ast::Literal<ast::Literals::BOOL>>
parse_literal_bool(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse the current token as a int literal
 *
 * @return The optional parsed int literal
 */
std::experimental::optional<ast::Literal<ast::Literals::INT>>
parse_literal_int(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse the current token as a double literal
 *
 * @return The optional parsed double literal
 */
std::experimental::optional<ast::Literal<ast::Literals::DOUBLE>>
parse_literal_double(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse the current token as a string literal
 *
 * @return The optional parsed string literal
 */
std::experimental::optional<ast::Literal<ast::Literals::STRING>>
parse_literal_string(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Definition parsing
//////////////////////////////////////////
/**
 * @param  token  The token
 *
 * @return True if keyword, False otherwise.
 */
bool is_keyword(const std::string& token);
/**
 * @brief  Expects that the current token is not a keyword
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 *
 * @throws UserSourceExc
 */
void expect_not_keyword(const Tokens& tokens, const size_t token);
/**
 * @brief  Parses function parameters ((foo,bar,...))
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  fun     The ast::Function object the parameters are added to
 */
template <typename T, typename is_Function<T>::type = false>
void parse_function_parameter(const Tokens& tokens, size_t& token, T& fun);
/**
 * @brief  Tries to parse the function internals (foo,bar ...){...})
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 * @param  fun            The ast::Function object the parameters are added to
 *
 * @return The parsed Function
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
template <typename T, typename is_Function<T>::type = false>
T parse_function_internals(const Tokens& tokens, size_t& token, T&& fun);
/**
 * @brief  Tries to parse a entry function (main(...){...})
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed function
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::callable::EntryFunction>
parse_entry_function(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse a entry function (fun(...){...})
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed function
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::callable::Function>
parse_function(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse a function definition  (def fun(...){...} / def
 *         main(...) {...})
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed definition
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Define>
parse_function_definition(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse a variable definition (var foo)
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed definition
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Define>
parse_variable_definition(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Callable parsing
//////////////////////////////////////////
/**
 * @brief  Expects a named parameter (foo:bar)
 *
 * @param  tokens         The tokens that are being parsed
 * @param  token          The current token that is being reported
 *
 * @throws UserSourceExc
 */
void expect_named_parameter(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse a named parameter list (foo:bar, baz:gun(), ...)
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed named parameter list
 *
 * @throws UserTailExc
 */
std::experimental::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse a function call (fun(foo:bar))
 *
 * @param  tokens       The tokens that are being parsed
 * @param  token        The current token that is being reported
 *
 * @return The optional parsed function call
 *
 * @throws UserTailExc
 */
std::experimental::optional<ast::callable::Callable>
parse_callable(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Scope parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parses a ast::Scope::Node
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  nodes   The nodes from the ast::Scope
 *
 * @return true if a ast::Scope::Node was parsed, else false

 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
bool parse_scope_internals(const Tokens& tokens, size_t& token,
                           std::vector<ast::Scope::Node>& nodes);
/**
 * @brief  Tries to parses all ast::Scope::Node available
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  scope   The ast::Scope to parse the nodes  for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope);
/**
 * @brief  Tries to parse a ast::Scope
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return the optional parsed ast::Scope
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Scope> parse_scope(const Tokens& tokens,
                                                    size_t& token);

//////////////////////////////////////////
/// Variable parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parse a ast::Variable
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Variable
 *
 * @throws UserTailExc
 */
std::experimental::optional<ast::Variable> parse_variable(const Tokens& tokens,
                                                          size_t& token);

//////////////////////////////////////////
/// Return parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parse a ast::callable;;Return
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::callable::Return
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::callable::Return>
parse_return(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Break parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parse a ast::loop::Break
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::loop::break
 */
std::experimental::optional<ast::loop::Break> parse_break(const Tokens& tokens,
                                                          size_t& token);
//////////////////////////////////////////
/// Continue parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parse a ast::loop::Continue
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::loop::Continue
 */
std::experimental::optional<ast::loop::Continue>
parse_continue(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Operator parsing
//////////////////////////////////////////
/**
 * @brief  Expects that the ast:.Operator has a type
 *
 * @param  file           The file name / macro name
 * @param  line           The line
 * @param  op             The ast::Operator to check
 *
 * @throws OperatorExc
 */
template <typename T, typename is_Operator<T>::type = false>
void expect_operator_type(const char* const file, const size_t line,
                          const T& op);
/**
 * @brief  Determine if the given node is a ValueProducer
 *
 * @param  node  The node to check
 *
 * @return true if value, false otherwise.
 *
 * @throws UserSourceExc
 */
bool is_value(const ast::Scope::Node& node);
/**
 * @brief  Converts the given node to a ast::ValueProducer
 *
 * @param  node  The node to convert
 *
 * @return The converted node
 *
 * @throws ConversionExc
 */
ast::ValueProducer node_to_value(ast::Scope::Node& node);
/**
 * @brief  Converts the given ValueProducer instance to a ast::Scope::Node
 *
 * @param  producer  The ValueProducer instance to convert
 *
 * @return The converted ValueProducer instance
 */
ast::Scope::Node value_to_node(ast::ValueProducer& producer);
/**
 * @brief  Converts the given node to an ast::Operator
 *
 * @param  tokens  The tokens that are being parsed
 * @param  node    The node to convert
 *
 * @return The converted node
 *
 * @throws ConversionExc
 */
ast::Operator node_to_operator(const Tokens& tokens, ast::Scope::Node& node);
/**
 * @brief  Extracts the defined ast::Variable from a ast::Definition
 *
 * @param  node  The node that may be a ast::Definition
 *
 * @return The optional ast::Definition
 */
std::experimental::optional<ast::Variable>
extract_var_def(ast::Scope::Node& node);
/**
 * @brief   Sets up the operator assemble workspace
 *
 * @details This method tries to extract a variable from a definition previously
 *          parsed or a previously parsed ast::ValueProducer and pushes it into
 *          the workspace vector, that way the operator can consume the
 *          ast::ValueProducer if needed.
 *
 * @param   workspace  The workspace that will be set up
 * @param   nodes      The previously parsed nodes
 * @param   op         The operator that will be assembled
 *
 *
 * @throws OperatorExc
 */
void setup_operator_wokespace(std::vector<ast::Scope::Node>& workspace,
                              std::vector<ast::Scope::Node>& nodes,
                              ast::Operator& op);
/**
 * @brief  Expects that the operator has enough operands to be assembled
 *
 * @param  token          The token of the Operator
 * @param  file           The file name / macro name
 * @param  previous       The previous node
 * @param  next           The next node
 * @param  end            The end end of the nodes vector
 *
 * @throws UserSourceExc
 */
void expect_operatees(const Token& token, const std::string& file,
                      const std::vector<ast::Scope::Node>::iterator& previous,
                      const std::vector<ast::Scope::Node>::iterator& next,
                      const std::vector<ast::Scope::Node>::iterator& end);
/**
 * @brief  Assembles the unary Operators
 *
 * @param  file      The file name / macro name
 * @param  nodes     The nodes that have to be assembled
 * @param  previous  The previous node
 * @param  next      The next node
 * @param  op        The current ast:.Operator that is about to be assembled
 *
 * @return true if the operator was assembled, else false
 */
bool assamble_unary(const std::string& file,
                    std::vector<ast::Scope::Node>& nodes,
                    std::vector<ast::Scope::Node>::iterator& previous,
                    std::vector<ast::Scope::Node>::iterator& next,
                    ast::Operator& op);
/**
 * @brief  Assembles the binary Operators
 *
 * @param  file      The file name / macro name
 * @param  nodes     The nodes that have to be assembled
 * @param  index     The index that marks the current position in the nodes
 *                   vector, after the assembly of a binary operator it will be
 *                   decremented. That __will__ invalidate all iterators!
 * @param  previous  The previous node
 * @param  next      The next node
 * @param  op        The current ast:.Operator that is about to be assembled
 *
 * @return true if the operator was assembled, else false
 */
void assamble_binary(const std::string& file,
                     std::vector<ast::Scope::Node>& nodes, size_t& index,
                     std::vector<ast::Scope::Node>::iterator& previous,
                     std::vector<ast::Scope::Node>::iterator& next,
                     ast::Operator& op);
/**
 * @brief  Assembles the given Operator with the given nodes
 *
 * @param  file   The file name / macro name
 * @param  nodes  The nodes that have to be assembled
 * @param  index  The index that marks the current position in the nodes vector,
 *                after the assembly of a binary operator it will be
 *                decremented. That __will__ invalidate all iterators!
 * @param  op     The current ast:.Operator that is about to be assembled
 */
void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes, size_t& index,
                       ast::Operator& op);
/**
 * @brief  Calls the given function with the nodes read from left to right (from
 *         top to bottom)
 *
 * @param  file   The file name / macro name
 * @param  nodes  The nodes that have to be assembled
 * @param  fun    The function to be executed at each ast::Operator to indicate
 *                if the operator should be assembled ([](ast::Operator& op) ->
 *                bool)
 */
template <typename FUN>
void assamble_operators_left_to_right(const std::string& file,
                                      std::vector<ast::Scope::Node>& nodes,
                                      FUN fun);
/**
 * @brief  Calls the given function with the nodes read from right to left (from
 *         bottom to top)
 *
 * @param  file   The file name / macro name
 * @param  nodes  The nodes that have to be assembled
 * @param  fun    The function to be executed at each ast::Operator to indicate
 *                if the operator should be assembled ([](ast::Operator& op) ->
 *                bool)
 */
template <typename FUN>
void assamble_operators_right_to_left(const std::string& file,
                                      std::vector<ast::Scope::Node>& nodes,
                                      FUN fun);
/**
 * @brief  Assembles all ast::Operator instances in the nodes vector
 *
 * @param  file   The file name / macro name
 * @param  nodes  The nodes that have to be assembled
 */
void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes);

/**
 * @brief  Tries to parse a unary ast::Operator
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Operator
 */
std::experimental::optional<ast::Operator>
parse_unary_operator(const Tokens& tokens, size_t& token);
/**
 * @brief  Parses the operantees
 *
 * @param  tokens     The tokens that are being parsed
 * @param  token      The current token that is being reported
 * @param  workspace  The workspace
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_operants(const Tokens& tokens, size_t& token,
                    std::vector<ast::Scope::Node>& workspace);
/**
 * @brief  Tries to parse a binary ast::Operator
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Operator
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Operator>
parse_binary_operator(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse the internal ast::Operator (unary or binary)
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Operator
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Operator>
parse_operator_internals(const Tokens& tokens, size_t& token);
/**
 * @brief  Tries to parse and assemble the ast::Operators
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Operator
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::Operator>
parse_operator(const Tokens& tokens, size_t& token,
               std::vector<ast::Scope::Node>& nodes);

//////////////////////////////////////////
/// Condition parsing
//////////////////////////////////////////
/**
 * @brief  Tries to parse a ast::Condition (An (binary) operator were the left
 *         hand side is not yet parsed )
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::Condition
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::ValueProducer>
parse_condition(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// If parsing
//////////////////////////////////////////
/**
 * @brief  Parses the if condition
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  iff     The if the condition is parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_if_condition(const Tokens& tokens, size_t& token,
                        ast::logic::If& iff);
/**
 * @brief  Parsed the true scope
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  iff     The if the condition is parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_true(const Tokens& tokens, size_t& token, ast::logic::If& iff);
/**
 * @brief  Parsed the false scope
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  iff     The if the condition is parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_false(const Tokens& tokens, size_t& token, ast::logic::If& iff);
/**
 * @brief  Tries to parse a ast::If (if/else)
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::logic::If
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::logic::If> parse_if(const Tokens& tokens,
                                                     size_t& token);

//////////////////////////////////////////
/// While parsing
//////////////////////////////////////////
/**
 * @brief  Parses the while condition
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  whi     The while the condition was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_while_condition(const Tokens& tokens, size_t& token,
                           ast::loop::While& whi);
/**
 * @brief  Parses the ast::While ast::Scope
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  whi     The while the condition was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_while_scope(const Tokens& tokens, size_t& token,
                       ast::loop::While& whi);
/**
 * @brief  Tries to parse a do-while
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::loop::While
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::loop::While> parse_while(const Tokens& tokens,
                                                          size_t& token);
/**
 * @brief  Tries to parse a do-while
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::loop::DoWhile
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::loop::DoWhile>
parse_do_while(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// For parsing
//////////////////////////////////////////
/**
 * @brief  Parses the variable / definition for the ast::For (for(x;y;z) x)
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  f       The for the variable / definition was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_for_variable(const Tokens& tokens, size_t& token, ast::loop::For& f);
/**
 * @brief  Parses the condition for the ast::For (for(x;y;z) y)
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  f       The for the condition was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_for_conditon(const Tokens& tokens, size_t& token, ast::loop::For& f);
/**
 * @brief  Parses the operation for the ast::For (for(x;y;z) z)
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  f       The for the operation was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_for_operation(const Tokens& tokens, size_t& token,
                         ast::loop::For& f);
/**
 * @brief  Parses the header for the ast::For - definition/variable (x),
 *         condition (y), operation (z) (for(x;y;z))
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 * @param  f       The for the operation was parsed for
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
void parse_for_header(const Tokens& tokens, size_t& token, ast::loop::For& f);
/**
 * @brief  Tries to parse a ast::For
 *
 * @param  tokens  The tokens that are being parsed
 * @param  token   The current token that is being reported
 *
 * @return The optional parsed ast::For
 *
 * @throws UserSourceExc
 * @throws UserTailExc
 * @throws OperatorExc
 * @throws ConversionExc
 */
std::experimental::optional<ast::loop::For> parse_for(const Tokens& tokens,
                                                      size_t& token);

//////////////////////////////////////////
/// Implementation
//////////////////////////////////////////

//////////////////////////////////////////
/// Exception
//////////////////////////////////////////
Token node_to_token(const ast::Scope::Node& node) {
  using namespace ast;
  Token token;

  eggs::match(
      node, [&token](const loop::Break& e) { token = e.token; },
      [&token](const loop::Continue& e) { token = e.token; },
      [&token](const Variable& e) { token = e.token; },
      [&token](const Define& e) { token = e.token; },
      [&token](const callable::Callable& e) { token = e.token; },
      [&token](const callable::Return& e) { token = e.token; },
      [&token](const Scope& e) { token = e.token; },
      [&token](const Operator& e) { token = e.token; },
      [&token](const logic::If& e) { token = e.token; },
      [&token](const loop::While& e) { token = e.token; },
      [&token](const loop::DoWhile& e) { token = e.token; },
      [&token](const loop::For& e) { token = e.token; },
      [&token](const Literal<Literals::BOOL>& e) { token = e.token; },
      [&token](const Literal<Literals::INT>& e) { token = e.token; },
      [&token](const Literal<Literals::DOUBLE>& e) { token = e.token; },
      [&token](const Literal<Literals::STRING>& e) { token = e.token; });
  return token;
}

template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun, const size_t arrow_position) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n'
      << *token.source_line << '\n'
      << std::string(arrow_position, ' ') << "^";
  }
}

template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun) {
  add_exception_info(token, file, e, fun, token.column - 1);
}

template <typename FUN>
void add_exception_info_end(const Token& token, const std::string& file,
                            UserExc& e, FUN fun) {
  add_exception_info(token, file, e, fun, token.column + token.token.size());
}

template <typename FUN>
void add_exception_info_end(const Tokens& tokens, const size_t token,
                            UserExc& e, FUN fun) {
  if(token >= tokens.size()) {
    add_exception_info_end(tokens.at(tokens.size() - 1), tokens.file, e, fun);
  } else {
    add_exception_info_end(tokens.at(token), tokens.file, e, fun);
  }
}
template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token, UserExc& e,
                        FUN fun) {
  if(token >= tokens.size()) {
    add_exception_info_end(tokens.at(tokens.size() - 1), tokens.file, e, fun);
  } else {
    add_exception_info(tokens.at(token), tokens.file, e, fun);
  }
}

[[noreturn]] void throw_unexprected_token(const Tokens& tokens,
                                          const size_t token) {
  UserSourceExc e;
  add_exception_info(tokens, token, e, [&] {
    e << "Unexpected token '" << tokens.at(token).token << '\'';
  });
  throw e;
}
[[noreturn]] void throw_unexprected_token(const Token& token,
                                          const std::string& file) {
  UserSourceExc e;
  add_exception_info(token, file, e,
                     [&] { e << "Unexpected token '" << token.token << '\''; });
  throw e;
}

[[noreturn]] void throw_conversion(const char* const file, const size_t line,
                                   const char* const prefix) {
  ConversionExc e(file, line, "Bad conversion");
  e << prefix << " is not convertible to a ValueProducer.";
  throw e;
}

void expect_no_space_between_bracket(const Tokens& tokens, const size_t token) {
  if(tokens.at(token).column + tokens.at(token).token.length() !=
     tokens.at(token + 1).column) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "There my not be any space between the function identifier and "
           "parentheses.";
    });
    throw e;
  }
}

//////////////////////////////////////////
/// Token reading
//////////////////////////////////////////
void expect_token(const Tokens& tokens, size_t& token,
                  const char* const token_literal) {
  if(token >= tokens.size() || tokens.at(token).token != token_literal) {
    UserSourceExc e;
    add_exception_info_end(tokens, token, e,
                           [&] { e << "Missing '" << token_literal << "'"; });
    throw e;
  }
  ++token;
}

bool read_token(const Tokens& tokens, size_t& token,
                const char* const token_literal) {
  if(token >= tokens.size() || tokens.at(token).token != token_literal) {
    return false;
  }
  ++token;
  return true;
}

bool read_token(const Tokens& tokens, size_t& token,
                const std::regex& token_regex) {
  std::smatch match;
  if(token >= tokens.size() ||
     !std::regex_match(tokens.at(token).token, match, token_regex)) {
    return false;
  }
  ++token;
  return true;
}

//////////////////////////////////////////
/// Literal parsing
//////////////////////////////////////////
std::experimental::optional<ast::Literal<ast::Literals::BOOL>>
parse_literal_bool(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "true")) {
    ast::Literal<ast::Literals::BOOL> lit(tokens.at(token));
    lit.data = true;

    token = tmp;
    return lit;
  } else if(read_token(tokens, tmp, "false")) {
    ast::Literal<ast::Literals::BOOL> lit(tokens.at(token));
    lit.data = false;

    token = tmp;
    return lit;
  }
  return {};
}

std::experimental::optional<ast::Literal<ast::Literals::INT>>
parse_literal_int(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([0-9]+)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::INT> lit(tokens.at(token));
    lit.data = std::stoi(tokens.at(token).token);

    token = tmp;
    return lit;
  }
  return {};
}

std::experimental::optional<ast::Literal<ast::Literals::DOUBLE>>
parse_literal_double(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([0-9]?.[0-9]+)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::DOUBLE> lit(tokens.at(token));
    lit.data = std::stod(lit.token.token);

    token = tmp;
    return lit;
  }
  return {};
}

void unescape_string(std::string& s) {
  auto replace_all = [&s](const std::string& search,
                          const std::string& replace) {
    for(size_t pos = 0;; pos += replace.length()) {
      pos = s.find(search, pos);

      if(pos == std::string::npos) {
        break;
      }
      s.erase(pos, search.length());
      s.insert(pos, replace);
    }
  };

  replace_all("\\/", "/");
  replace_all("\\\"", "\"");
  replace_all("\\\\", "\\");
  replace_all("\\b", "\b");
  replace_all("\\f", "\f");
  replace_all("\\n", "\n");
  replace_all("\\r", "\r");
  replace_all("\\t", "\t");
  replace_all("\\a", "\a");
}

std::experimental::optional<ast::Literal<ast::Literals::STRING>>
parse_literal_string(const Tokens& tokens, size_t& token) {
  const static std::regex regex("(\\\".*\\\")");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::STRING> lit(tokens.at(token));
    lit.data = lit.token.token.substr(1, lit.token.token.size() - 2);

    unescape_string(lit.data);

    token = tmp;
    return lit;
  }
  return {};
}

//////////////////////////////////////////
/// Definition parsing
//////////////////////////////////////////
bool is_keyword(const std::string& token) {
  for(const auto& w : keywords) {
    if(token == w) {
      return true;
    }
  }
  return false;
}

void expect_not_keyword(const Tokens& tokens, const size_t token) {
  if(is_keyword(tokens.at(token).token)) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "'" << tokens.at(token).token
        << "' is a keyword an may not be used as qualifier.";
    });
    throw e;
  }
}

template <typename T, typename is_Function<T>::type>
void parse_function_parameter(const Tokens& tokens, size_t& token, T& fun) {
  while(auto var = parse_variable(tokens, token)) {
    fun.parameter.push_back(std::move(*var));
    if(!read_token(tokens, token, ",")) {
      break;  // we do not expect another variable
    }
  }
}

template <typename T, typename is_Function<T>::type>
T parse_function_internals(const Tokens& tokens, size_t& token, T&& fun) {
  auto tmp = token;

  parse_function_parameter(tokens, tmp, fun);
  expect_token(tokens, tmp, ")");

  auto fun_scope = parse_scope(tokens, tmp);
  if(!fun_scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  fun.scope = std::make_unique<ast::Scope>(*std::move(fun_scope));
  token = tmp;

  return fun;
}

std::experimental::optional<ast::callable::EntryFunction>
parse_entry_function(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, "main")) {
      expect_token(tokens, tmp, "(");
      auto fun = parse_function_internals(
          tokens, tmp, ast::callable::EntryFunction(tokens.at(token)));
      token = tmp;
      return fun;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the 'main' function defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}

std::experimental::optional<ast::callable::Function>
parse_function(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
      expect_not_keyword(tokens, token);
      expect_no_space_between_bracket(tokens, token);

      auto fun = parse_function_internals(
          tokens, tmp, ast::callable::Function(tokens.at(token)));

      token = tmp;
      return fun;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "In the '" << tokens.at(token).token << "' function defined here";
    });
    std::throw_with_nested(e);
  }
  return {};
}

std::experimental::optional<ast::Define>
parse_function_definition(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "def")) {
    ast::Define def(tokens.at(token));

    if(auto entry_function = parse_entry_function(tokens, tmp)) {
      def.definition = std::move(*entry_function);
    } else if(auto function = parse_function(tokens, tmp)) {
      def.definition = std::move(*function);
    } else {
      throw_unexprected_token(tokens, tmp);
    }
    token = tmp;
    return def;
  }
  return {};
}

std::experimental::optional<ast::Define>
parse_variable_definition(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, "var")) {
      ast::Define def(tokens.at(token));

      if(auto variable = parse_variable(tokens, tmp)) {
        def.definition = std::move(*variable);
      } else {
        throw_unexprected_token(tokens, tmp);
      }
      token = tmp;
      return def;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "At the '" << tokens.at(token).token << "' variable defined here";
    });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Callable parsing
//////////////////////////////////////////
void expect_named_parameter(const Tokens& tokens, size_t& token) {
  if(!read_token(tokens, token, ":")) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "Expected a ':' after '" << tokens.at(token - 1).token
        << "' followed by an expression as value.";
    });
    throw e;
  }
}

std::experimental::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(auto fun_var = parse_variable(tokens, tmp)) {
    expect_named_parameter(tokens, tmp);
    std::pair<ast::Variable, ast::ValueProducer> ret;

    if(auto con = parse_condition(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*con));
    } else if(auto exe = parse_callable(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*var));
    } else {
      UserSourceExc e;
      add_exception_info(tokens, tmp, e, [&] {
        e << "Expected an expression, but found this unexpected token '"
          << tokens.at(tmp).token << '\'';
      });
      throw e;
    }
    token = tmp;
    return ret;
  }
  return {};
}

std::experimental::optional<ast::callable::Callable>
parse_callable(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");

  auto tmp = token;
  try {
    if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
      expect_no_space_between_bracket(tokens, token);

      ast::callable::Callable call(tokens.at(token));

      while(tmp < tokens.size()) {
        if(auto param = parse_callable_parameter(tokens, tmp)) {
          call.parameter.emplace_back(std::move(*param));

          if(!read_token(tokens, tmp, ",")) {
            break;  // we do not expect another variable
          }
        } else {
          break;  // the exception comes from the parenthesis check
        }
      }
      expect_token(tokens, tmp, ")");
      token = tmp;
      return call;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "In the function call '" << tokens.at(token).token
        << "' defined here";
    });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Scope parsing
//////////////////////////////////////////
bool parse_scope_internals(const Tokens& tokens, size_t& token,
                           std::vector<ast::Scope::Node>& nodes) {
  ast::Scope::Node node;

  auto expect_end = [&tokens, &token]() {
    try {
      expect_token(tokens, token, ";");
    } catch(UserExc&) {
      UserTailExc e;
      add_exception_info(tokens, token - 1, e, [&e] { e << "Expected a ';'"; });
      throw e;
    }
  };

  if(auto br = parse_break(tokens, token)) {
    expect_end();
    nodes.push_back(std::move(*br));
  } else if(auto con = parse_continue(tokens, token)) {
    expect_end();
    nodes.push_back(std::move(*con));
  } else if(auto def = parse_function_definition(tokens, token)) {
    nodes.push_back(std::move(*def));
  } else if(auto var_def = parse_variable_definition(tokens, token)) {
    nodes.push_back(std::move(*var_def));
    if(auto op = parse_operator(tokens, token, nodes)) {
      if(op->operation != ast::Operation::ASSIGNMENT) {
        throw_unexprected_token(op->token, tokens.file);
      }
      nodes.push_back(std::move(*op));
    }
    expect_end();
  } else if(auto iff = parse_if(tokens, token)) {
    nodes.push_back(std::move(*iff));
  } else if(auto whi = parse_while(tokens, token)) {
    nodes.push_back(std::move(*whi));
  } else if(auto dwhi = parse_do_while(tokens, token)) {
    nodes.push_back(std::move(*dwhi));
    expect_end();
  } else if(auto foor = parse_for(tokens, token)) {
    nodes.push_back(std::move(*foor));
  } else if(auto ret = parse_return(tokens, token)) {
    nodes.push_back(std::move(*ret));
    expect_end();
  } else if(auto condition = parse_condition(tokens, token)) {
    nodes.push_back(value_to_node(*condition));
    expect_end();
  } else if(auto call = parse_callable(tokens, token)) {
    nodes.push_back(std::move(*call));
    expect_end();
  } else if(auto lit_bool = parse_literal_bool(tokens, token)) {
    nodes.push_back(std::move(*lit_bool));
    expect_end();
  } else if(auto lit_int = parse_literal_int(tokens, token)) {
    nodes.push_back(std::move(*lit_int));
    expect_end();
  } else if(auto lit_double = parse_literal_double(tokens, token)) {
    nodes.push_back(std::move(*lit_double));
    expect_end();
  } else if(auto lit_string = parse_literal_string(tokens, token)) {
    nodes.push_back(std::move(*lit_string));
    expect_end();
  } else if(auto scope = parse_scope(tokens, token)) {
    nodes.push_back(std::move(*scope));
  } else if(auto var = parse_variable(tokens, token)) {
    nodes.push_back(std::move(*var));
    if(auto op = parse_operator(tokens, token, nodes)) {
      nodes.push_back(std::move(*op));
    }
    expect_end();
  } else if(read_token(tokens, token, ";")) {
  } else {
    return false;
  }
  return true;
}

void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope) {

  while(token < tokens.size()) {
    if(!parse_scope_internals(tokens, token, scope.nodes)) {
      break;  // done with the scope
    }
  }
}

std::experimental::optional<ast::Scope> parse_scope(const Tokens& tokens,
                                                    size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "{")) {
    ast::Scope sub_scope(tokens.at(token));

    parse_scope_internals(tokens, tmp, sub_scope);
    expect_token(tokens, tmp, "}");

    token = tmp;
    return sub_scope;
  }
  return {};
}

//////////////////////////////////////////
/// Variable parsing
//////////////////////////////////////////
std::experimental::optional<ast::Variable> parse_variable(const Tokens& tokens,
                                                          size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    expect_not_keyword(tokens, token);

    ast::Variable var(Token(tokens.at(token)));
    token = tmp;
    return var;
  }
  return {};
}

//////////////////////////////////////////
/// Return parsing
//////////////////////////////////////////
std::experimental::optional<ast::callable::Return>
parse_return(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, "return")) {
      ast::callable::Return ret(tokens.at(token));

      if(auto con = parse_condition(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*con));
      } else if(auto exe = parse_callable(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*exe));
      } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_bool));
      } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_int));
      } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
        ret.output =
            std::make_unique<ast::ValueProducer>(std::move(*lit_double));
      } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
        ret.output =
            std::make_unique<ast::ValueProducer>(std::move(*lit_string));
      } else if(auto var = parse_variable(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*var));
      } else {
        throw_unexprected_token(tokens, tmp);
      }

      token = tmp;
      return ret;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "At return defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Break parsing
//////////////////////////////////////////
std::experimental::optional<ast::loop::Break> parse_break(const Tokens& tokens,
                                                          size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "break")) {
    ast::loop::Break ret(tokens.at(token));

    token = tmp;
    return ret;
  }
  return {};
}

//////////////////////////////////////////
/// Continue parsing
//////////////////////////////////////////
std::experimental::optional<ast::loop::Continue>
parse_continue(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "continue")) {
    ast::loop::Continue ret(tokens.at(token));

    token = tmp;
    return ret;
  }
  return {};
}

//////////////////////////////////////////
/// Operator parsing
//////////////////////////////////////////
template <typename T, typename is_Operator<T>::type>
void expect_operator_type(const char* const file, const size_t line,
                          const T& op) {
  if(op.operation == ast::Operation::NONE) {
    OperatorExc e(file, line, "Missing operator");
    e << "There was no type specified for the operator:\n" << op;
    throw e;
  }
}

ast::ValueProducer node_to_value(ast::Scope::Node& node) {
  using namespace ast;
  using namespace callable;
  ast::ValueProducer value;

  eggs::match(
      node, [&value](Variable& e) { value = std::move(e); },
      [&value](Callable& e) { value = std::move(e); },
      [&value](Operator& e) { value = std::move(e); },
      [&value](Literal<Literals::BOOL>& e) { value = std::move(e); },
      [&value](Literal<Literals::INT>& e) { value = std::move(e); },
      [&value](Literal<Literals::DOUBLE>& e) { value = std::move(e); },
      [&value](Literal<Literals::STRING>& e) { value = std::move(e); },
      [](loop::Break&) { throw_conversion(__FILE__, __LINE__, "Break"); },
      [](loop::Continue&) { throw_conversion(__FILE__, __LINE__, "Continue"); },
      [](Define&) { throw_conversion(__FILE__, __LINE__, "Define"); },
      [](Return&) { throw_conversion(__FILE__, __LINE__, "Return"); },
      [](Scope&) { throw_conversion(__FILE__, __LINE__, "Scope"); },
      [](logic::If&) { throw_conversion(__FILE__, __LINE__, "If"); },
      [](loop::While&) { throw_conversion(__FILE__, __LINE__, "While"); },
      [](loop::DoWhile&) { throw_conversion(__FILE__, __LINE__, "DoWhile"); },
      [](loop::For&) { throw_conversion(__FILE__, __LINE__, "For"); });
  return value;
}

ast::Operator node_to_operator(const Tokens& tokens, ast::Scope::Node& node) {
  using namespace ast;

  ast::Operator ret;
  std::experimental::optional<Token> token;

  eggs::match(node,
              [&ret](Operator& op) {
                expect_operator_type(__FILE__, __LINE__, op);
                ret = std::move(op);
              },
              [&token](loop::Break& ele) { token = ele.token; },
              [&token](loop::Continue& ele) { token = ele.token; },
              [&token](Variable& ele) { token = ele.token; },
              [&token](Define& ele) { token = ele.token; },
              [&token](callable::Callable& ele) { token = ele.token; },
              [&token](callable::Return& ele) { token = ele.token; },
              [&token](Scope& ele) { token = ele.token; },
              [&token](logic::If& ele) { token = ele.token; },
              [&token](loop::While& ele) { token = ele.token; },
              [&token](loop::DoWhile& ele) { token = ele.token; },
              [&token](loop::For& ele) { token = ele.token; },
              [&token](Literal<Literals::BOOL>& ele) { token = ele.token; },
              [&token](Literal<Literals::INT>& ele) { token = ele.token; },
              [&token](Literal<Literals::DOUBLE>& ele) { token = ele.token; },
              [&token](Literal<Literals::STRING>& ele) { token = ele.token; });
  if(token) {
    throw_unexprected_token(*token, tokens.file);
  }
  return ret;
}

ast::Scope::Node value_to_node(ast::ValueProducer& producer) {
  ast::Scope::Node node;
  eggs::match(
      producer.value, [&node](ast::Variable& e) { node = std::move(e); },
      [&node](ast::callable::Callable& e) { node = std::move(e); },
      [&node](ast::Operator& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::BOOL>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::INT>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::DOUBLE>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::STRING>& e) { node = std::move(e); });
  return node;
}

std::experimental::optional<ast::Variable>
extract_var_def(ast::Scope::Node& node) {
  std::experimental::optional<ast::Variable> ret;

  eggs::match(node,
              [&ret](const ast::Define& def) {
                eggs::match(def.definition,
                            [&ret](const ast::Variable& var) { ret = var; },
                            [](const ast::callable::Function&) {},
                            [](const ast::callable::EntryFunction&) {});

              },                                                  //
              [](const ast::loop::Break&) {},                     //
              [](const ast::loop::Continue&) {},                  //
              [](const ast::Variable&) {},                        //
              [](const ast::callable::Callable&) {},              //
              [](const ast::callable::Return&) {},                //
              [](const ast::Scope&) {},                           //
              [](const ast::Operator&) {},                        //
              [](const ast::logic::If&) {},                       //
              [](const ast::loop::While&) {},                     //
              [](const ast::loop::DoWhile&) {},                   //
              [](const ast::loop::For&) {},                       //
              [](const ast::Literal<ast::Literals::BOOL>&) {},    //
              [](const ast::Literal<ast::Literals::INT>&) {},     //
              [](const ast::Literal<ast::Literals::DOUBLE>&) {},  //
              [](const ast::Literal<ast::Literals::STRING>&) {});
  return ret;
}

void setup_operator_wokespace(std::vector<ast::Scope::Node>& workspace,
                              std::vector<ast::Scope::Node>& nodes,
                              ast::Operator& op) {
  expect_operator_type(__FILE__, __LINE__, op);

  if(nodes.size() > 0) {
    if(auto var = extract_var_def(nodes.back())) {
      workspace.push_back(std::move(*var));
    } else {
      workspace.push_back(std::move(nodes.back()));
      nodes.erase(nodes.end() - 1);
    }
  }
  workspace.push_back(std::move(op));
}

void parse_operants(const Tokens& tokens, size_t& token,
                    std::vector<ast::Scope::Node>& workspace) {
  auto tmp = token;

  while(tmp < tokens.size()) {
    if(read_token(tokens, tmp, "(")) {
      if(auto con = parse_condition(tokens, tmp)) {
        workspace.push_back(value_to_node(*con));
        expect_token(tokens, tmp, ")");
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected an expression."; });
        throw e;
      }
    } else if(auto op = parse_operator_internals(tokens, tmp)) {
      workspace.push_back(std::move(*op));
    } else if(auto exe = parse_callable(tokens, tmp)) {
      workspace.push_back(std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      workspace.push_back(std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      workspace.push_back(std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      workspace.push_back(std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      workspace.push_back(std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      workspace.push_back(std::move(*var));
    } else {
      break;  // We are done
    }
  }

  token = tmp;
}

void expect_operatees(const Token& token, const std::string& file,
                      const std::vector<ast::Scope::Node>::iterator& previous,
                      const std::vector<ast::Scope::Node>::iterator& next,
                      const std::vector<ast::Scope::Node>::iterator& end) {
  if(previous == end && next == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing left and right hand token for binary operator '"
        << token.token << '\'';
    });
    throw e;
  }
  if(previous == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing left hand token for binary operator '" << token.token
        << '\'';
    });
    throw e;
  }
  if(next == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing right hand token for binary operator '" << token.token
        << '\'';
    });
    throw e;
  }
}

bool is_value(const ast::Scope::Node& node) {
  using namespace ast;

  bool ret = false;

  eggs::match(node, [&ret](const Operator&) { ret = false; },
              [&ret](const loop::Break&) { ret = false; },
              [&ret](const loop::Continue&) { ret = false; },
              [&ret](const Variable&) { ret = true; },
              [&ret](const Define&) { ret = false; },
              [&ret](const callable::Callable&) { ret = true; },
              [&ret](const callable::Return&) { ret = false; },
              [&ret](const Scope&) { ret = false; },
              [&ret](const logic::If&) { ret = false; },
              [&ret](const loop::While&) { ret = false; },
              [&ret](const loop::DoWhile&) { ret = false; },
              [&ret](const loop::For&) { ret = false; },
              [&ret](const Literal<Literals::BOOL>&) { ret = true; },
              [&ret](const Literal<Literals::INT>&) { ret = true; },
              [&ret](const Literal<Literals::DOUBLE>&) { ret = true; },
              [&ret](const Literal<Literals::STRING>&) { ret = true; });

  return ret;
}

bool assamble_unary(const std::string& file,
                    std::vector<ast::Scope::Node>& nodes,
                    std::vector<ast::Scope::Node>::iterator& previous,
                    std::vector<ast::Scope::Node>::iterator& next,
                    ast::Operator& op) {
  if(next == nodes.end()) {
    UserSourceExc e;
    add_exception_info(op.token, file, e, [&] {
      e << "Missing token for unary operator '" << op.token.token << '\'';
    });
    throw e;
  }

  if(op.operation == ast::Operation::NEGATIVE && previous != nodes.end() &&
     is_value(*previous)) {
    op.operation = ast::Operation::SUBTRACT;
    return true;
  } else if(op.operation == ast::Operation::POSITIVE &&
            previous != nodes.end() && is_value(*previous)) {
    op.operation = ast::Operation::ADD;
    return true;
  }

  if(op.operation == ast::Operation::NOT ||
     op.operation == ast::Operation::PRINT ||
     op.operation == ast::Operation::TYPEOF ||
     op.operation == ast::Operation::NEGATIVE ||
     op.operation == ast::Operation::POSITIVE) {
    op.right_operand =
        std::make_unique<ast::ValueProducer>(node_to_value(*next));
    nodes.erase(next);
    return true;
  }
  return false;
}

void assamble_binary(const std::string& file,
                     std::vector<ast::Scope::Node>& nodes, size_t& index,
                     std::vector<ast::Scope::Node>::iterator& previous,
                     std::vector<ast::Scope::Node>::iterator& next,
                     ast::Operator& op) {
  expect_operatees(op.token, file, previous, next, nodes.end());

  op.left_operand =
      std::make_unique<ast::ValueProducer>(node_to_value(*previous));
  op.right_operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));

  --index;  // we used the left
  nodes.erase(next);
  // We changed the vector - get new, VALID iterator
  previous = nodes.begin();
  std::advance(previous, index);  // we decremented index already
  nodes.erase(previous);
}

void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes, size_t& index,
                       ast::Operator& op) {
  auto next = nodes.begin();
  std::advance(next, index + 1);
  auto previous = nodes.begin();
  if(index == 0) {
    previous = nodes.end();
  } else {
    std::advance(previous, index - 1);
  }

  if(!assamble_unary(file, nodes, previous, next, op)) {
    assamble_binary(file, nodes, index, previous, next, op);
  }
}

template <typename FUN>
void assamble_operators_left_to_right(const std::string& file,
                                      std::vector<ast::Scope::Node>& nodes,
                                      FUN fun) {
  for(size_t i = 0; i < nodes.size(); ++i) {
    eggs::match(nodes.at(i),
                [&file, &nodes, &i, &fun](ast::Operator& op) {
                  expect_operator_type(__FILE__, __LINE__, op);

                  if(fun(op) && !op.left_operand && !op.right_operand) {
                    assamble_operator(file, nodes, i, op);
                  }
                },
                [](ast::loop::Break&) {},                      //
                [](ast::loop::Continue&) {},                   //
                [](ast::Variable&) {},                         //
                [](ast::Define&) {},                           //
                [](ast::callable::Callable&) {},               //
                [](ast::callable::Return&) {},                 //
                [](ast::Scope&) {},                            //
                [](ast::logic::If&) {},                        //
                [](ast::loop::While&) {},                      //
                [](ast::loop::DoWhile&) {},                    //
                [](ast::loop::For&) {},                        //
                [](ast::Literal<ast::Literals::BOOL>&) {},     //
                [](ast::Literal<ast::Literals::INT>&) {},      //
                [](ast::Literal<ast::Literals::DOUBLE>&) {},   //
                [](ast::Literal<ast::Literals::STRING>&) {});  //
  }
}

template <typename FUN>
void assamble_operators_right_to_left(const std::string& file,
                                      std::vector<ast::Scope::Node>& nodes,
                                      FUN fun) {
  auto i = nodes.size();
  if(i > 0)
    do {
      --i;
      eggs::match(nodes.at(i),
                  [&file, &nodes, &i, &fun](ast::Operator& op) {
                    expect_operator_type(__FILE__, __LINE__, op);

                    if(fun(op) && !op.left_operand && !op.right_operand) {
                      assamble_operator(file, nodes, i, op);
                    }
                  },
                  [](ast::loop::Break&) {},                      //
                  [](ast::loop::Continue&) {},                   //
                  [](ast::Variable&) {},                         //
                  [](ast::Define&) {},                           //
                  [](ast::callable::Callable&) {},               //
                  [](ast::callable::Return&) {},                 //
                  [](ast::Scope&) {},                            //
                  [](ast::logic::If&) {},                        //
                  [](ast::loop::While&) {},                      //
                  [](ast::loop::DoWhile&) {},                    //
                  [](ast::loop::For&) {},                        //
                  [](ast::Literal<ast::Literals::BOOL>&) {},     //
                  [](ast::Literal<ast::Literals::INT>&) {},      //
                  [](ast::Literal<ast::Literals::DOUBLE>&) {},   //
                  [](ast::Literal<ast::Literals::STRING>&) {});  //
    } while(i != 0);
}

void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes) {
  assamble_operators_right_to_left(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::NEGATIVE ||
           op.operation == ast::Operation::POSITIVE;
  });
  assamble_operators_right_to_left(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::NOT;
  });
  assamble_operators_right_to_left(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::TYPEOF;
  });

  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::DIVIDE ||
           op.operation == ast::Operation::MULTIPLY ||
           op.operation == ast::Operation::MODULO;
  });
  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::ADD ||
           op.operation == ast::Operation::SUBTRACT;
  });

  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::SMALLER ||
           op.operation == ast::Operation::SMALLER_EQUAL ||
           op.operation == ast::Operation::GREATER ||
           op.operation == ast::Operation::GREATER_EQUAL;
  });
  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::EQUAL ||
           op.operation == ast::Operation::NOT_EQUAL;
  });
  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::AND;
  });
  assamble_operators_left_to_right(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::OR;
  });

  assamble_operators_right_to_left(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::PRINT;
  });
  assamble_operators_right_to_left(file, nodes, [](const ast::Operator& op) {
    return op.operation == ast::Operation::ASSIGNMENT;
  });

  if(nodes.size() > 1) {
    throw_unexprected_token(node_to_token(nodes.at(1)), file);
  }
}

std::experimental::optional<ast::Operator>
parse_unary_operator(const Tokens& tokens, size_t& token) {
  auto op = ast::Operation::NONE;
  auto tmp = token;

  if(read_token(tokens, tmp, "!")) {
    op = ast::Operation::NOT;
  } else if(read_token(tokens, tmp, "typeof")) {
    op = ast::Operation::TYPEOF;
  } else if(read_token(tokens, tmp, "print")) {
    op = ast::Operation::PRINT;
  } else if(read_token(tokens, tmp, "-")) {
    op = ast::Operation::NEGATIVE;
  } else if(read_token(tokens, tmp, "+")) {
    op = ast::Operation::POSITIVE;
  }

  if(op != ast::Operation::NONE) {
    ast::Operator un(tokens.at(token));
    un.operation = op;
    token = tmp;
    return un;
  }
  return {};
}
std::experimental::optional<ast::Operator>
parse_binary_operator(const Tokens& tokens, size_t& token) {
  auto op = ast::Operation::NONE;
  auto tmp = token;

  if(read_token(tokens, tmp, "/")) {
    op = ast::Operation::DIVIDE;
  } else if(read_token(tokens, tmp, "*")) {
    op = ast::Operation::MULTIPLY;
  } else if(read_token(tokens, tmp, "%")) {
    op = ast::Operation::MODULO;
  } else if(read_token(tokens, tmp, "+")) {
    op = ast::Operation::ADD;  // This is not reachable - see unary
  } else if(read_token(tokens, tmp, "-")) {
    op = ast::Operation::SUBTRACT;  // This is not reachable - see unary
  } else if(read_token(tokens, tmp, "<")) {
    op = ast::Operation::SMALLER;
  } else if(read_token(tokens, tmp, "<=")) {
    op = ast::Operation::SMALLER_EQUAL;
  } else if(read_token(tokens, tmp, ">")) {
    op = ast::Operation::GREATER;
  } else if(read_token(tokens, tmp, ">=")) {
    op = ast::Operation::GREATER_EQUAL;
  } else if(read_token(tokens, tmp, "==")) {
    op = ast::Operation::EQUAL;
  } else if(read_token(tokens, tmp, "!=")) {
    op = ast::Operation::NOT_EQUAL;
  } else if(read_token(tokens, tmp, "&&")) {
    op = ast::Operation::AND;
  } else if(read_token(tokens, tmp, "||")) {
    op = ast::Operation::OR;
  } else if(read_token(tokens, tmp, "=")) {
    op = ast::Operation::ASSIGNMENT;
  }

  if(op != ast::Operation::NONE) {
    ast::Operator bi(tokens.at(token));
    bi.operation = op;
    token = tmp;
    return bi;
  }
  return {};
}

std::experimental::optional<ast::Operator>
parse_operator_internals(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  std::experimental::optional<ast::Operator> ret;

  if(auto un = parse_unary_operator(tokens, tmp)) {
    ret.emplace(std::move(*un));
    token = tmp;
  } else if(auto bi = parse_binary_operator(tokens, tmp)) {
    ret.emplace(std::move(*bi));
    token = tmp;
  }
  return ret;
}

std::experimental::optional<ast::Operator>
parse_operator(const Tokens& tokens, size_t& token,
               std::vector<ast::Scope::Node>& nodes) {
  auto tmp = token;
  std::experimental::optional<ast::Operator> ret;

  try {
    std::vector<ast::Scope::Node> workspace;

    if(auto op = parse_operator_internals(tokens, tmp)) {
      setup_operator_wokespace(workspace, nodes, *op);
      parse_operants(tokens, tmp, workspace);
      assamble_operator(tokens.file, workspace);

      if(workspace.size() == 1) {
        ret = node_to_operator(tokens, workspace.front());
      }
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "At the operator '" << tokens.at(token).token << "' defined here";
    });
    std::throw_with_nested(e);
  }
  if(ret) {
    token = tmp;
  }
  return ret;
}

//////////////////////////////////////////
/// Condition parsing
//////////////////////////////////////////
std::experimental::optional<ast::ValueProducer>
parse_condition(const Tokens& tokens, size_t& token) {
  std::vector<ast::Scope::Node> conditions;
  auto tmp = token;

  while(tmp < tokens.size()) {
    if(read_token(tokens, tmp, "(")) {
      if(auto condition = parse_condition(tokens, tmp)) {
        conditions.push_back(value_to_node(*condition));
        expect_token(tokens, tmp, ")");
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected an expression."; });
        throw e;
      }
    } else if(auto op = parse_operator(tokens, tmp, conditions)) {
      conditions.push_back(std::move(*op));
    } else if(auto exe = parse_callable(tokens, tmp)) {
      conditions.push_back(std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      conditions.push_back(std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      conditions.push_back(std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      conditions.push_back(std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      conditions.push_back(std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      conditions.push_back(std::move(*var));
    } else {
      break;  // We are done
    }
  }
  if(!conditions.empty()) {
    assamble_operator(tokens.file, conditions);
    auto condition = node_to_value(conditions.front());
    token = tmp;
    return condition;
  } else {
    return {};
  }
}

//////////////////////////////////////////
/// If parsing
//////////////////////////////////////////
void parse_if_condition(const Tokens& tokens, size_t& token,
                        ast::logic::If& iff) {
  auto tmp = token;

  expect_token(tokens, tmp, "(");
  auto condition = parse_condition(tokens, tmp);
  if(!condition) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected an expression."; });
    throw e;
  }

  expect_token(tokens, tmp, ")");

  token = tmp;
  iff.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
}

void parse_true(const Tokens& tokens, size_t& token, ast::logic::If& iff) {
  auto tmp = token;

  auto true_scope = parse_scope(tokens, tmp);

  if(!true_scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  token = tmp;
  iff.true_scope = std::make_unique<ast::Scope>(std::move(*true_scope));
}

void parse_false(const Tokens& tokens, size_t& token, ast::logic::If& iff) {
  auto tmp = token;

  if(read_token(tokens, tmp, "else")) {
    try {
      auto scope = parse_scope(tokens, tmp);

      if(!scope) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
        throw e;
      }

      token = tmp;
      iff.false_scope = std::make_unique<ast::Scope>(std::move(*scope));
    } catch(UserExc&) {
      UserTailExc e;
      add_exception_info(tokens, token, e,
                         [&e] { e << "In the else part defined here"; });
      std::throw_with_nested(e);
    }
  }
}

std::experimental::optional<ast::logic::If> parse_if(const Tokens& tokens,
                                                     size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "if")) {
      ast::logic::If iff(tokens.at(token));

      parse_if_condition(tokens, tmp, iff);
      parse_true(tokens, tmp, iff);

      if(tmp < tokens.size()) {
        parse_false(tokens, tmp, iff);
      }
      token = tmp;
      return iff;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the if defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// While parsing
//////////////////////////////////////////
void parse_while_condition(const Tokens& tokens, size_t& token,
                           ast::loop::While& whi) {
  auto tmp = token;

  expect_token(tokens, tmp, "(");
  auto condition = parse_condition(tokens, tmp);
  if(!condition) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a condition."; });
    throw e;
  }
  whi.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
  expect_token(tokens, tmp, ")");
  token = tmp;
}

void parse_while_scope(const Tokens& tokens, size_t& token,
                       ast::loop::While& whi) {
  auto tmp = token;

  auto scope = parse_scope(tokens, tmp);
  if(!scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  whi.scope = std::make_unique<ast::Scope>(std::move(*scope));
  token = tmp;
}

std::experimental::optional<ast::loop::While> parse_while(const Tokens& tokens,
                                                          size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "while")) {
      ast::loop::While w(tokens.at(token));

      parse_while_condition(tokens, tmp, w);
      parse_while_scope(tokens, tmp, w);
      token = tmp;
      return w;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the while defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}
std::experimental::optional<ast::loop::DoWhile>
parse_do_while(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "do")) {
      ast::loop::DoWhile w(tokens.at(token));

      parse_while_scope(tokens, tmp, w);
      expect_token(tokens, tmp, "while");
      parse_while_condition(tokens, tmp, w);
      token = tmp;
      return w;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the do-while defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}

void parse_for_variable(const Tokens& tokens, size_t& token,
                        ast::loop::For& f) {
  auto tmp = token;

  if(auto def = parse_variable_definition(tokens, tmp)) {
    std::vector<ast::Scope::Node> op_wip;
    eggs::match(
        def->definition, [&op_wip](ast::Variable& e) { op_wip.push_back(e); },
        [](ast::callable::Function&) { assert(false && "Can't happen."); });
    f.define = std::move(*def);
    if(auto op = parse_operator(tokens, tmp, op_wip)) {
      f.variable = {std::move(*op)};
    }
    token = tmp;
  } else if(auto con = parse_condition(tokens, tmp)) {
    f.variable = std::move(*con);
    token = tmp;
  }
}
void parse_for_conditon(const Tokens& tokens, size_t& token,
                        ast::loop::For& f) {
  auto tmp = token;

  if(auto con = parse_condition(tokens, tmp)) {
    f.condition = std::make_unique<ast::ValueProducer>(std::move(*con));
    token = tmp;
  }
}
void parse_for_operation(const Tokens& tokens, size_t& token,
                         ast::loop::For& f) {
  auto tmp = token;

  if(auto con = parse_condition(tokens, tmp)) {
    f.operation = std::move(*con);
    token = tmp;
  }
}
void parse_for_header(const Tokens& tokens, size_t& token, ast::loop::For& f) {

  auto tmp = token;
  expect_token(tokens, tmp, "(");
  parse_for_variable(tokens, tmp, f);
  expect_token(tokens, tmp, ";");
  parse_for_conditon(tokens, tmp, f);
  expect_token(tokens, tmp, ";");
  parse_for_operation(tokens, tmp, f);
  expect_token(tokens, tmp, ")");
  token = tmp;
}

std::experimental::optional<ast::loop::For> parse_for(const Tokens& tokens,
                                                      size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "for")) {
      ast::loop::For f(tokens.at(token));

      parse_for_header(tokens, tmp, f);
      parse_while_scope(tokens, tmp, f);

      token = tmp;
      return f;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the do-while defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}
}

ast::Scope parse(std::string macro, std::string file_name) {
  Tokens tokens = {tokenizer::tokenize(macro), file_name};
  auto root = ast::Scope(Token(0, 0, ""));

  for(size_t i = 0; i < tokens.size(); ++i) {
    parse_scope_internals(tokens, i, root);
  }
  Analyser ana(file_name);
  auto messages = ana.analyse(root);

  if(messages.size() > 0) {
    UserTailExc exc;
    for(const auto& s : messages) {
      for(const auto& m : s) {
        exc << m.message();
      }
    }
    throw exc;
  }

  return root;
}
}
}
}
