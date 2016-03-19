#ifndef cad_macro_ast_literal_Int_h
#define cad_macro_ast_literal_Int_h

#include "cad/macro/ast/AST.h"

#include <limits>
#include <cmath>

namespace cad {
namespace macro {
namespace ast {
enum class Literals { BOOL, INT, DOUBLE, STRING };

template <Literals T>
class Literal : public AST {
  using DataType = typename std::conditional_t<
      T == Literals::BOOL, bool,
      typename std::conditional_t<
          T == Literals::INT, int,
          typename std::conditional_t<T == Literals::DOUBLE, double,
                                      std::string>>>;

  constexpr auto literal_name() const {
    switch(T) {
    case Literals::BOOL:
      return "Boolean";
    case Literals::INT:
      return "Integer";
    case Literals::DOUBLE:
      return "Double";
    case Literals::STRING:
      return "String";
    }
    return "not possible";
  }

protected:
  template <bool SFINAE = true,
            typename std::enable_if<T == Literals::BOOL && SFINAE, bool>::type =
                false>
  void print_internals(IndentStream& os) const {
    os << "Data:\n";
    os.indent() << std::boolalpha << data << "\n";
    os.dedent() << std::noboolalpha;
  }

  template <bool SFINAE = true,
            typename std::enable_if<T != Literals::BOOL && SFINAE, bool>::type =
                false>
  void print_internals(IndentStream& os) const {
    os << "Data:\n";
    os.indent() << data << "\n";
    os.dedent();
  }

public:
  DataType data;

  Literal() = default;
  Literal(parser::Token token)
      : AST(std::move(token))
      , data(DataType()) {
  }

  template <bool SFINAE = true,
            typename std::enable_if<T == Literals::DOUBLE && SFINAE,
                                    bool>::type = false>
  bool operator==(const Literal& other) const {
    if(this == &other) {
      return true;
    } else if(AST::operator==(other)) {
      return data == other.data ||
             std::abs(data - other.data) <
                 std::abs(std::min(data, other.data)) *
                     std::numeric_limits<double>::epsilon();
    }
    return false;
  }

  template <bool SFINAE = true,
            typename std::enable_if<T != Literals::DOUBLE && SFINAE,
                                    bool>::type = false>
  bool operator==(const Literal& other) const {
    if(this == &other) {
      return true;
    } else if(AST::operator==(other)) {
      return data == other.data;
    }
    return false;
  }

  bool operator!=(const Literal& other) const {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os, const Literal& ast) {
    ast.print_token(os, ast.literal_name(),
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}

#endif
