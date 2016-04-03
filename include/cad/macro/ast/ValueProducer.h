#ifndef cad_macro_ast_ValueProducer_h
#define cad_macro_ast_ValueProducer_h

#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/Variable.h"

#include <core/variant.hpp>

namespace cad {
namespace macro {
namespace ast {
class ValueProducer {
  using ValueVariant =
      ::core::variant<callable::Callable, Variable, Literal<Literals::BOOL>,
                      Literal<Literals::INT>, Literal<Literals::DOUBLE>,
                      Literal<Literals::STRING>, UnaryOperator, BinaryOperator>;

public:
  ValueVariant value;

public:
  ValueProducer() = default;
  ValueProducer(ValueVariant op);
  ValueProducer(callable::Callable op);
  ValueProducer(Literal<Literals::BOOL> op);
  ValueProducer(Literal<Literals::INT> op);
  ValueProducer(Literal<Literals::DOUBLE> op);
  ValueProducer(Literal<Literals::STRING> op);
  ValueProducer(Variable op);
  ValueProducer(UnaryOperator op);
  ValueProducer(BinaryOperator op);

  bool operator==(const ValueProducer& other) const;
  bool operator!=(const ValueProducer& other) const;

  friend std::ostream& operator<<(std::ostream& os, const ValueProducer& op) {
    op.value.match([&os](const callable::Callable& o) { os << o; },
                   [&os](const Variable& o) { os << o; },
                   [&os](const UnaryOperator& o) { os << o; },
                   [&os](const BinaryOperator& o) { os << o; },
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
