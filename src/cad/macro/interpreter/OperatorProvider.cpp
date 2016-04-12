#include "cad/macro/interpreter/OperatorProvider.h"

#include <exception.h>

#include <cassert>

namespace cad {
namespace macro {
namespace interpreter {
namespace {
template <typename T1, typename T2>
using VecMap = std::vector<std::pair<T1, T2>>;

template <typename T1, typename T2>
auto find(VecMap<T1, T2>& map, const T1& key) {
  return std::find_if(map.begin(), map.end(),
                      [&key](const auto& pair) { return pair.first == key; });
}

template <typename T1, typename T2>
auto find(const VecMap<T1, T2>& map, const T1& key) {
  return std::find_if(map.begin(), map.end(),
                      [&key](const auto& pair) { return pair.first == key; });
}

template <typename T1, typename T2>
auto exists(const VecMap<T1, T2>& map, const T1& key) {
  return find(map, key) != map.end();
}

auto make_indexes(const ::core::any& lhs, const ::core::any& rhs) {
  return std::make_tuple(std::type_index(lhs.type()),
                         std::type_index(rhs.type()));
}
}

void OperatorProvider::add(const BinaryOperation operati, std::type_index lhs,
                           std::type_index rhs, BiOp operato) {
  switch(operati) {
  case BinaryOperation::DIVIDE:
    if(exists(divide_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'divide'(/) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    divide_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::MULTIPLY:
    if(exists(multiply_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'multiply'(*) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    multiply_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::MODULO:
    if(exists(modulo_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'modulo'(%) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    modulo_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::ADD:
    if(exists(add_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'add'(+) already exists for the types '" << lhs.name()
        << "' and '" << rhs.name() << "'.";
      throw e;
    }
    add_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::SUBTRACT:
    if(exists(subtract_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'subtract'(-) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    subtract_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::SMALLER:
    if(exists(smaller_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'smaller'(<) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    smaller_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::SMALLER_EQUAL:
    if(exists(smaller_equal_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'smaller_equal'(<=) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    smaller_equal_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::GREATER:
    if(exists(greater_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'greater'(>) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    greater_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::GREATER_EQUAL:
    if(exists(greater_equal_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'greater_equal'(>=) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    greater_equal_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::EQUAL:
    if(exists(equal_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'equal'(==) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    equal_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::NOT_EQUAL:
    if(exists(not_equal_, std::make_tuple(lhs, rhs))) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'not_equal'(!=) already exists for the types '"
        << lhs.name() << "' and '" << rhs.name() << "'.";
      throw e;
    }
    not_equal_.emplace_back(std::make_tuple(lhs, rhs), std::move(operato));
    break;
  case BinaryOperation::AND:
    assert(false && "AND can not be added - it is free :)");
  case BinaryOperation::OR:
    assert(false && "OR can not be added - it is free :)");
  }
}
void OperatorProvider::add(const UnaryOperation operati, std::type_index rhs,
                           UnOp operato) {
  switch(operati) {
  case UnaryOperation::NOT:
    assert(false && "NOT can not be added - it is free :)");
  case UnaryOperation::BOOL:
    if(exists(bool_, rhs)) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'bool' already exists for the type '" << rhs.name()
        << "'.";
      throw e;
    }
    bool_.emplace_back(rhs, std::move(operato));
    break;
  case UnaryOperation::TYPEOF:
    if(exists(type_of_, rhs)) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'typeof' already exists for the type '" << rhs.name()
        << "'.";
      throw e;
    }
    type_of_.emplace_back(rhs, std::move(operato));
    break;
  case UnaryOperation::PRINT:
    if(exists(print_, rhs)) {
      Exc<E, E::OPERATOR_EXISTS> e(__FILE__, __LINE__, "Operator exists");
      e << "The operator 'print' already exists for the type '" << rhs.name()
        << "'.";
      throw e;
    }
    print_.emplace_back(rhs, std::move(operato));
  }
}

bool OperatorProvider::has(const BinaryOperation op, const ::core::any& lhs,
                           const ::core::any& rhs) const {
  switch(op) {
  case BinaryOperation::DIVIDE:
    return exists(divide_, make_indexes(lhs, rhs));
  case BinaryOperation::MULTIPLY:
    return exists(multiply_, make_indexes(lhs, rhs));
  case BinaryOperation::MODULO:
    return exists(modulo_, make_indexes(lhs, rhs));
  case BinaryOperation::ADD:
    return exists(add_, make_indexes(lhs, rhs));
  case BinaryOperation::SUBTRACT:
    return exists(subtract_, make_indexes(lhs, rhs));
  case BinaryOperation::SMALLER:
    return exists(smaller_, make_indexes(lhs, rhs));
  case BinaryOperation::SMALLER_EQUAL:
    return exists(smaller_equal_, make_indexes(lhs, rhs));
  case BinaryOperation::GREATER:
    return exists(greater_, make_indexes(lhs, rhs));
  case BinaryOperation::GREATER_EQUAL:
    return exists(greater_equal_, make_indexes(lhs, rhs));
  case BinaryOperation::EQUAL:
    return exists(equal_, make_indexes(lhs, rhs));
  case BinaryOperation::NOT_EQUAL:
    return exists(not_equal_, make_indexes(lhs, rhs));
  case BinaryOperation::AND:
    return exists(bool_, std::type_index(lhs.type())) &&
           exists(bool_, std::type_index(rhs.type()));
  case BinaryOperation::OR:
    return exists(bool_, std::type_index(lhs.type())) &&
           exists(bool_, std::type_index(rhs.type()));
  }
  assert(false && "Reached by access after free and similar");
}
bool OperatorProvider::has(const BinaryOperation op, const std::type_index& lhs,
                           const std::type_index& rhs) const {
  switch(op) {
  case BinaryOperation::DIVIDE:
    return exists(divide_, std::make_tuple(lhs, rhs));
  case BinaryOperation::MULTIPLY:
    return exists(multiply_, std::make_tuple(lhs, rhs));
  case BinaryOperation::MODULO:
    return exists(modulo_, std::make_tuple(lhs, rhs));
  case BinaryOperation::ADD:
    return exists(add_, std::make_tuple(lhs, rhs));
  case BinaryOperation::SUBTRACT:
    return exists(subtract_, std::make_tuple(lhs, rhs));
  case BinaryOperation::SMALLER:
    return exists(smaller_, std::make_tuple(lhs, rhs));
  case BinaryOperation::SMALLER_EQUAL:
    return exists(smaller_equal_, std::make_tuple(lhs, rhs));
  case BinaryOperation::GREATER:
    return exists(greater_, std::make_tuple(lhs, rhs));
  case BinaryOperation::GREATER_EQUAL:
    return exists(greater_equal_, std::make_tuple(lhs, rhs));
  case BinaryOperation::EQUAL:
    return exists(equal_, std::make_tuple(lhs, rhs));
  case BinaryOperation::NOT_EQUAL:
    return exists(not_equal_, std::make_tuple(lhs, rhs));
  case BinaryOperation::AND:
    return exists(bool_, lhs) && exists(bool_, rhs);
  case BinaryOperation::OR:
    return exists(bool_, lhs) && exists(bool_, rhs);
  }
  assert(false && "Reached by access after free and similar");
}

bool OperatorProvider::has(const UnaryOperation op,
                           const std::type_index& rhs) const {
  switch(op) {
  case UnaryOperation::NOT:
    return exists(bool_, rhs);
  case UnaryOperation::BOOL:
    return exists(bool_, rhs);
  case UnaryOperation::TYPEOF:
    return exists(type_of_, rhs);
  case UnaryOperation::PRINT:
    return exists(print_, rhs);
  }
  assert(false && "Reached by access after free and similar");
}
bool OperatorProvider::has(const UnaryOperation op,
                           const ::core::any& rhs) const {
  switch(op) {
  case UnaryOperation::NOT:
    return exists(bool_, std::type_index(rhs.type()));
  case UnaryOperation::BOOL:
    return exists(bool_, std::type_index(rhs.type()));
  case UnaryOperation::TYPEOF:
    return exists(type_of_, std::type_index(rhs.type()));
  case UnaryOperation::PRINT:
    return exists(print_, std::type_index(rhs.type()));
  }
  assert(false && "Reached by access after free and similar");
}

//////////////////////////////////////////
/// Binary
//////////////////////////////////////////
::core::any OperatorProvider::eval_divide(const ::core::any& lhs,
                                          const ::core::any& rhs) const {
  auto it = find(divide_, make_indexes(lhs, rhs));
  if(it != divide_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'divide'(/) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_multiply(const ::core::any& lhs,
                                            const ::core::any& rhs) const {
  auto it = find(multiply_, make_indexes(lhs, rhs));
  if(it != multiply_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'multiply'(*) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_modulo(const ::core::any& lhs,
                                          const ::core::any& rhs) const {
  auto it = find(modulo_, make_indexes(lhs, rhs));
  if(it != modulo_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'modulo'(%) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_add(const ::core::any& lhs,
                                       const ::core::any& rhs) const {
  auto it = find(add_, make_indexes(lhs, rhs));
  if(it != add_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'add'(+) is missing for the types '" << lhs.type().name()
    << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_subtract(const ::core::any& lhs,
                                            const ::core::any& rhs) const {
  auto it = find(subtract_, make_indexes(lhs, rhs));
  if(it != subtract_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'subtract'(-) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_smaller(const ::core::any& lhs,
                                           const ::core::any& rhs) const {
  auto it = find(smaller_, make_indexes(lhs, rhs));
  if(it != smaller_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'smaller'(<) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_smaller_equal(const ::core::any& lhs,
                                                 const ::core::any& rhs) const {
  auto it = find(smaller_equal_, make_indexes(lhs, rhs));
  if(it != smaller_equal_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'smaller_equal'(<=) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_greater(const ::core::any& lhs,
                                           const ::core::any& rhs) const {
  auto it = find(greater_, make_indexes(lhs, rhs));
  if(it != greater_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'greater'(>) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_greater_equal(const ::core::any& lhs,
                                                 const ::core::any& rhs) const {
  auto it = find(greater_equal_, make_indexes(lhs, rhs));
  if(it != greater_equal_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'greater_equal'(>=) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_equal(const ::core::any& lhs,
                                         const ::core::any& rhs) const {
  auto it = find(equal_, make_indexes(lhs, rhs));
  if(it != equal_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'equal'(==) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_not_equal(const ::core::any& lhs,
                                             const ::core::any& rhs) const {
  auto it = find(not_equal_, make_indexes(lhs, rhs));
  if(it != not_equal_.end()) {
    return it->second(lhs, rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'not_equal'(!=) is missing for the types '"
    << lhs.type().name() << "' and '" << rhs.type().name() << "'.";
  throw e;
}
::core::any OperatorProvider::eval_and(const ::core::any& lhs,
                                       const ::core::any& rhs) const {
  auto to_bool = [this](const ::core::any& var) {
    if(var.type() == typeid(bool)) {  // no need to convert
      return ::core::any_cast<bool>(var);
    } else {  // need to convert
      auto b = eval(UnaryOperation::BOOL, var);

      if(b.type() == typeid(bool)) {
        return ::core::any_cast<bool>(b);
      } else {  // not a bool type ...
        Exc<E, E::BAD_BOOL_CAST> e(__FILE__, __LINE__, "Bad bool cast");
        e << "Tried cast '" << var.type().name()
          << "' to bool but the operator returned '" << b.type().name() << "'.";
        throw e;
      }
    }
  };

  return to_bool(lhs) && to_bool(rhs);
}
::core::any OperatorProvider::eval_or(const ::core::any& lhs,
                                      const ::core::any& rhs) const {
  auto to_bool = [this](const ::core::any& var) {
    if(var.type() == typeid(bool)) {  // no need to convert
      return ::core::any_cast<bool>(var);
    } else {  // need to convert
      auto b = eval(UnaryOperation::BOOL, var);

      if(b.type() == typeid(bool)) {
        return ::core::any_cast<bool>(b);
      } else {  // not a bool type ...
        Exc<E, E::BAD_BOOL_CAST> e(__FILE__, __LINE__, "Bad bool cast");
        e << "Tried cast '" << var.type().name()
          << "' to bool but the operator returned '" << b.type().name() << "'.";
        throw e;
      }
    }
  };

  return to_bool(lhs) || to_bool(rhs);
}
::core::any OperatorProvider::eval(const BinaryOperation op,
                                   const ::core::any& lhs,
                                   const ::core::any& rhs) const {
  switch(op) {
  case BinaryOperation::DIVIDE:
    return eval_divide(lhs, rhs);
  case BinaryOperation::MULTIPLY:
    return eval_multiply(lhs, rhs);
  case BinaryOperation::MODULO:
    return eval_modulo(lhs, rhs);
  case BinaryOperation::ADD:
    return eval_add(lhs, rhs);
  case BinaryOperation::SUBTRACT:
    return eval_subtract(lhs, rhs);
  case BinaryOperation::SMALLER:
    return eval_smaller(lhs, rhs);
  case BinaryOperation::SMALLER_EQUAL:
    return eval_smaller_equal(lhs, rhs);
  case BinaryOperation::GREATER:
    return eval_greater(lhs, rhs);
  case BinaryOperation::GREATER_EQUAL:
    return eval_greater_equal(lhs, rhs);
  case BinaryOperation::EQUAL:
    return eval_equal(lhs, rhs);
  case BinaryOperation::NOT_EQUAL:
    return eval_not_equal(lhs, rhs);
  case BinaryOperation::AND:
    return eval_and(lhs, rhs);
  case BinaryOperation::OR:
    return eval_or(lhs, rhs);
  }
  assert(false && "Reached by access after free and similar");
}

//////////////////////////////////////////
/// Unary
//////////////////////////////////////////
::core::any OperatorProvider::eval_not(const ::core::any& rhs) const {
  auto b_rhs = eval(UnaryOperation::BOOL, rhs);
  if(b_rhs.type() == typeid(bool)) {
    return !::core::any_cast<bool>(b_rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'bool' is missing for the type '" << rhs.type().name()
    << "'.";
  throw e;
}
::core::any OperatorProvider::eval_bool(const ::core::any& rhs) const {
  auto it = find(bool_, std::type_index(rhs.type()));
  if(it != bool_.end()) {
    return it->second(rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'bool' is missing for the type '" << rhs.type().name()
    << "'.";
  throw e;
}
::core::any OperatorProvider::eval_type_of(const ::core::any& rhs) const {
  auto it = find(type_of_, std::type_index(rhs.type()));
  if(it != type_of_.end()) {
    return it->second(rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'bool' is missing for the type '" << rhs.type().name()
    << "'.";
  throw e;
}
::core::any OperatorProvider::eval_print(const ::core::any& rhs) const {
  auto it = find(print_, std::type_index(rhs.type()));
  if(it != print_.end()) {
    return it->second(rhs);
  }
  Exc<E, E::MISSING_OPERATOR> e(__FILE__, __LINE__, "Missing Operator");
  e << "The operator 'bool' is missing for the type '" << rhs.type().name()
    << "'.";
  throw e;
}
::core::any OperatorProvider::eval(const UnaryOperation op,
                                   const ::core::any& rhs) const {
  switch(op) {
  case UnaryOperation::NOT:
    return eval_not(rhs);
  case UnaryOperation::BOOL:
    return eval_bool(rhs);
  case UnaryOperation::TYPEOF:
    return eval_type_of(rhs);
  case UnaryOperation::PRINT:
    return eval_print(rhs);
  }
  assert(false && "Reached by access after free and similar");
}

OperatorProvider::OperatorProvider(const bool initialize) {
  using BiOp = BinaryOperation;
  using UnOp = UnaryOperation;

  if(initialize) {
    // BINARY
    add<int, int, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::MODULO, BiOp::ADD,
        BiOp::SUBTRACT, BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER,
        BiOp::GREATER_EQUAL, BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<bool, int, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::MODULO, BiOp::ADD,
        BiOp::SUBTRACT, BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER,
        BiOp::GREATER_EQUAL, BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<int, bool, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<double, double, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<bool, double, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<double, bool, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<int, double, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<double, int, BiOp::DIVIDE, BiOp::MULTIPLY, BiOp::ADD, BiOp::SUBTRACT,
        BiOp::SMALLER, BiOp::SMALLER_EQUAL, BiOp::GREATER, BiOp::GREATER_EQUAL,
        BiOp::EQUAL, BiOp::NOT_EQUAL>();
    add<std::string, std::string, BiOp::ADD, BiOp::SMALLER, BiOp::SMALLER_EQUAL,
        BiOp::GREATER, BiOp::GREATER_EQUAL, BiOp::EQUAL, BiOp::NOT_EQUAL>();

    add(BiOp::ADD, std::type_index(typeid(std::string)),
        std::type_index(typeid(bool)),
        [](const ::core::any& a, const ::core::any& b) {
          std::stringstream ss;
          ss << ::core::any_cast<std::string>(a) << std::boolalpha
             << ::core::any_cast<bool>(b);
          return ss.str();
        });
    add(BiOp::ADD, std::type_index(typeid(std::string)),
        std::type_index(typeid(int)),
        [](const ::core::any& a, const ::core::any& b) {
          std::stringstream ss;
          ss << ::core::any_cast<std::string>(a) << ::core::any_cast<int>(b);
          return ss.str();
        });
    add(BiOp::ADD, std::type_index(typeid(std::string)),
        std::type_index(typeid(double)),
        [](const ::core::any& a, const ::core::any& b) {
          std::stringstream ss;
          ss << ::core::any_cast<std::string>(a) << ::core::any_cast<double>(b);
          return ss.str();
        });

    // UNARY
    // BOOL
    add<bool, UnOp::BOOL>();
    add<int, UnOp::BOOL>();
    add<double, UnOp::BOOL>();
    add(UnOp::BOOL, std::type_index(typeid(std::string)),
        [](const ::core::any& a) {
          return ::core::any_cast<std::string>(a).size() > 0;
        });
    add(UnOp::BOOL, std::type_index(typeid(void)),
        [](const ::core::any&) { return false; });

    // TYPEOF
    add(UnOp::TYPEOF, std::type_index(typeid(bool)),
        [](const ::core::any&) { return std::string("bool"); });
    add(UnOp::TYPEOF, std::type_index(typeid(int)),
        [](const ::core::any&) { return std::string("int"); });
    add(UnOp::TYPEOF, std::type_index(typeid(double)),
        [](const ::core::any&) { return std::string("double"); });
    add(UnOp::TYPEOF, std::type_index(typeid(std::string)),
        [](const ::core::any&) { return std::string("string"); });
    add(UnOp::TYPEOF, std::type_index(typeid(void)),
        [](const ::core::any&) { return std::string("none"); });

    // PRINT
    add(UnOp::PRINT, std::type_index(typeid(bool)),
        [this](const ::core::any& a) { return eval_add(std::string(), a); });
    add(UnOp::PRINT, std::type_index(typeid(int)),
        [this](const ::core::any& a) { return eval_add(std::string(), a); });
    add(UnOp::PRINT, std::type_index(typeid(double)),
        [this](const ::core::any& a) { return eval_add(std::string(), a); });
    add(UnOp::PRINT, std::type_index(typeid(std::string)),
        [](const ::core::any& a) { return a; });
    add(UnOp::PRINT, std::type_index(typeid(void)),
        [this](const ::core::any&) { return std::string("none"); });
  }
}
}
}
}
