#ifndef cad_macro_ast_ValueProducer_h
#define cad_macro_ast_ValueProducer_h

#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/Variable.h"

#include <eggs/variant.hpp>

namespace cad {
namespace macro {
namespace ast {
/**
 * @brief  The ValueProducer is a convenience struct that wraps a variant of all
 *         ast structs that produce a value.
 */
struct ValueProducer {
private:
  using ValueVariant =
      eggs::variant<callable::Callable, Variable, Literal<Literals::BOOL>,
                    Literal<Literals::INT>, Literal<Literals::DOUBLE>,
                    Literal<Literals::STRING>, Operator>;

public:
  ValueVariant value;

public:
  /**
   * @brief  Ctor
   */
  ValueProducer() = default;
  /**
   * @brief  Ctor
   *
   * @param  op ValueProducer to represent
   */
  ValueProducer(ValueVariant op);
  /**
   * @brief  Ctor
   *
   * @param  op Callable to represent
   */
  ValueProducer(callable::Callable op);
  /**
   * @brief  Ctor
   *
   * @param  op Literal to represent
   */
  ValueProducer(Literal<Literals::BOOL> op);
  /**
   * @brief  Ctor
   *
   * @param  op Literal to represent
   */
  ValueProducer(Literal<Literals::INT> op);
  /**
   * @brief  Ctor
   *
   * @param  op Literal to represent
   */
  ValueProducer(Literal<Literals::DOUBLE> op);
  /**
   * @brief  Ctor
   *
   * @param  op Literal to represent
   */
  ValueProducer(Literal<Literals::STRING> op);
  /**
   * @brief  Ctor
   *
   * @param  op Variable to represent
   */
  ValueProducer(Variable op);
  /**
   * @brief  Ctor
   *
   * @param  op Operator to represent
   */
  ValueProducer(Operator op);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const ValueProducer& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const ValueProducer& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const ValueProducer& ast) {
    eggs::match(ast.value, [&os](const callable::Callable& o) { os << o; },
                [&os](const Variable& o) { os << o; },
                [&os](const Operator& o) { os << o; },
                [&os](const Literal<Literals::BOOL>& c) { os << c; },
                [&os](const Literal<Literals::INT>& c) { os << c; },
                [&os](const Literal<Literals::DOUBLE>& c) { os << c; },
                [&os](const Literal<Literals::STRING>& c) { os << c; });
    return os;
  }
};
}
}
}
#endif
