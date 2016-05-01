#ifndef cad_macro_ast_AST_h
#define cad_macro_ast_AST_h

#include "cad/macro/parser/Token.h"
#include "cad/macro/IndentStream.h"

#include <ostream>

namespace cad {
namespace macro {
namespace ast {
/**
 * @brief   AST is the base struct for the abstract syntax tree.
 *
 * @details This struct is used by the Parser, Analyser and Interpreter. The AST
 *          struct itself should not be seen in the tree itself as it resembles
 *          no element from the syntax.
 */
struct AST {
protected:
  /**
   * @brief  Prints the Token information with a given prefix for the calling
   *         struct name and a custom function that can print anything it wants.
   *
   * @param  os      output stream
   * @param  prefix  The prefix before the information
   * @param  fun     The function to be executed after the Token information.
   *                 The function will be given an IndentStream (std::ostream)
   *                 to stream its information to.
   */
  template <typename T>
  void print_token(std::ostream& os, std::string prefix, T fun) const {
    IndentStream indent_os(os);

    indent_os << '@' << prefix << " {\n";
    indent_os.indent() << "line: " << token.line << " column: " << token.column
                       << " token: " << token.token << "\n";
    fun(indent_os);
    indent_os.dedent() << "}\n";
  }

  /**
   * @brief   Overload function for the templated print_token function.
   * @details No additional informations will be added.
   *
   * @param   os      output stream
   * @param   prefix  The prefix
   */
  void print_token(std::ostream& os, std::string prefix) const;

public:
  parser::Token token;

  /**
   * @brief   Ctor
   */
  AST();
  /**
   * @brief   Ctor
   *
   * @param   token  The token of this node
   */
  AST(parser::Token token);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const AST& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const AST& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    stream to print the node into
   *
   * @return the input stream
   */
  std::ostream& operator<<(std::ostream& os);
};
}
}
}
#endif
