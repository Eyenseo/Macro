#ifndef cad_macro_ast_literal_Int_h
#define cad_macro_ast_literal_Int_h

#include "cad/macro/ast/AST.h"

#include <limits>
#include <cmath>

namespace cad {
namespace macro {
namespace ast {
enum class Literals { BOOL, INT, DOUBLE, STRING };

/**
 * @brief   The Literal struct represents all Literals from the macro
 * @details The class has the following Template specialisations
 * - true Literals::BOOL
 * - false Literals::BOOL
 * - 1 is Literals::INT
 * - .1 is Literals::DOUBLE
 * - 0.1 is Literals::DOUBLE
 * - "" is Literals::STRING
 *
 * @tparam  T     Type of the Literal
 */
template <Literals T>
struct Literal : public AST {
  using DataType = typename std::conditional_t<
      T == Literals::BOOL, bool,
      typename std::conditional_t<
          T == Literals::INT, int,
          typename std::conditional_t<T == Literals::DOUBLE, double,
                                      std::string>>>;

  /**
   * @brief  The function converts the template argument to a printable string
   *
   * @return name of the template argument
   */
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
  /**
   * @brief   Pretty prints the internals of this struct
   * @details This function is specialised for booleans
   *
   * @param   os         stream to print the internals to
   */
  template <bool SFINAE = true,
            typename std::enable_if<T == Literals::BOOL && SFINAE, bool>::type =
                false>
  void print_internals(IndentStream& os) const {
    os << "Data:\n";
    os.indent() << std::boolalpha << data << "\n";
    os.dedent() << std::noboolalpha;
  }

  /**
   * @brief  Pretty prints the internals of this struct
   * @details This function is for types except booleans
   *
   * @param  os         stream to print the internals to
   */
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

  /**
   * @brief  Ctor
   */
  Literal() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Literal(parser::Token token)
      : AST(std::move(token))
      , data(DataType()) {
  }

  /**
   * @brief  Equality comparison
   * @details This function is specialised for doubles
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
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
  /**
   * @brief  Equality comparison
   * @details This function is for types except doubles
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
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
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Literal& other) const {
    return !(*this == other);
  }

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
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
