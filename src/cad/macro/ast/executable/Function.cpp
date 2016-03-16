#include "cad/macro/ast/executable/Function.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
namespace {
// https://isocpp.org/files/papers/N3656.txt
template <class T>
struct UniqueIf {
  typedef std::unique_ptr<T> SingleObject;
};

template <class T>
struct UniqueIf<T[]> {
  typedef std::unique_ptr<T[]> UnknownBound;
};

template <class T, size_t N>
struct UniqueIf<T[N]> {
  typedef void KnownBound;
};

template <class T, class... Args>
typename UniqueIf<T>::SingleObject make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename UniqueIf<T>::UnknownBound make_unique(size_t n) {
  typedef typename std::remove_extent<T>::type U;
  return std::unique_ptr<T>(new U[n]());
}

template <class T, class... Args>
typename UniqueIf<T>::KnownBound make_unique(Args&&...) = delete;
}

Function::Function() {
}
Function::Function(parser::Token token)
    : Executable(std::move(token)) {
}
Function::Function(const Function& other)
    : Executable(other)
    , scope((other.scope) ? make_unique<Scope>(*other.scope) : nullptr) {
}
Function::~Function() {
}

void Function::print_internals(IndentStream& os) const {
  Executable::print_internals(os);
  if(scope) {
    os << *scope;
  }
}
bool Function::operator==(const Function& other) const {
  if(this == &other) {
    return true;
  } else if(Executable::operator==(other)) {
    if(scope && other.scope) {
      return *scope == *other.scope;
    } else if(!scope && !other.scope) {
      return true;
    }
  }
  return false;
}
bool Function::operator!=(const Function& other) const {
  return !(*this == other);
}
bool operator==(const Function& first, const AST& ast) {
  auto second = dynamic_cast<const Function*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
}
