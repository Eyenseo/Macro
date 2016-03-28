#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
ValueProducer::ValueProducer(ValueVariant op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(callable::Callable op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(Literal<Literals::BOOL> op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(Literal<Literals::INT> op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(Literal<Literals::DOUBLE> op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(Literal<Literals::STRING> op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(Variable op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(UnaryOperator op)
    : value(std::move(op)) {
}
ValueProducer::ValueProducer(BinaryOperator op)
    : value(std::move(op)) {
}

bool ValueProducer::operator==(const ValueProducer& other) const {
  if(this == &other) {
    return true;
  } else {
    return value == other.value;
  }
}
bool ValueProducer::operator!=(const ValueProducer& other) const {
  return !(*this == other);
}
}
}
}
