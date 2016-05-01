#ifndef cad_macro_ast_Operator_h
#define cad_macro_ast_Operator_h

#include "cad/macro/ast/AST.h"

#include <memory>

namespace cad {
namespace macro {
namespace ast {
struct ValueProducer;
}
}
}

namespace cad {
namespace macro {
namespace ast {
enum class Operation {
  NONE,
  DIVIDE,
  MULTIPLY,
  MODULO,
  ADD,
  SUBTRACT,
  SMALLER,
  SMALLER_EQUAL,
  GREATER,
  GREATER_EQUAL,
  EQUAL,
  NOT_EQUAL,
  AND,
  OR,
  ASSIGNMENT,
  NOT,
  PRINT,
  TYPEOF,
  NEGATIVE,
  POSITIVE
};

/**
 * @brief   The Operator represents all operator expressions in the macro.
 *
 * @details Possible OperatorTypes are:
 *  - Unary
 *    -# +
 *    -# -
 *    -# !
 *    -# print
 *    -# typeof
 *  - Binary
 *    -# *
 *    -# /
 *    -# %
 *    -# +
 *    -# -
 *    -# <
 *    -# <=
 *    -# >
 *    -# >=
 *    -# ==
 *    -# !=
 *    -# &&
 *    -# ||
 *    -# =
 */
struct Operator : public AST {
private:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> left_operand;
  std::unique_ptr<ValueProducer> right_operand;
  Operation operation;

public:
  /**
   * @brief  Ctor
   */
  Operator() = default;
  /**
   * @brief  CCtor
   */
  Operator(const Operator& other);
  /**
   * @brief  MCtor
   */
  Operator(Operator&& other);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Operator(parser::Token token);

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(Operator& first, Operator& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.left_operand, second.left_operand);
    swap(first.right_operand, second.right_operand);
    swap(first.operation, second.operation);
  }
  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  Operator& operator=(Operator other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Operator& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Operator& other) const;

  /**
   * @brief  converts the operation enum value to a printable string
   *
   * @return string of the enum value
   */
  std::string operation_to_string() const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Operator& ast) {
    if(ast.left_operand && ast.right_operand) {
      ast.print_token(os, "BinaryOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    } else if(ast.left_operand || ast.right_operand) {
      ast.print_token(os, "UnaryOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    } else {
      ast.print_token(os, "EmptyOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    }
    return os;
  }
};
}
}
}
#endif
