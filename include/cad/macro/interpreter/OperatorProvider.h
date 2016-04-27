#ifndef cad_macro_interpreter_OperatorProvider_h
#define cad_macro_interpreter_OperatorProvider_h

#include <functional>

#include <any.hpp>
#include <typeindex>
#include <vector>

namespace cad {
namespace macro {
namespace interpreter {
class OperatorProvider {
public:
  enum class UnaryOperation { NOT, BOOL, PRINT, TYPEOF, NEGATIVE, POSITIVE };
  enum class BinaryOperation {
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
    OR
  };
  enum class E { MISSING_OPERATOR, OPERATOR_EXISTS, BAD_BOOL_CAST };

protected:
  template <typename LHS, typename RHS, BinaryOperation op>
  struct BiHelper;
  template <typename RHS, UnaryOperation op>
  struct UnHelper;

  template <bool...>
  struct bool_pack;
  template <bool... v>
  using none = std::is_same<bool_pack<false, v...>, bool_pack<v..., false>>;

  template <typename T, T O1, T O2>
  struct Same
      : std::conditional<O1 == O2, std::true_type, std::false_type>::type {};
  template <BinaryOperation O1, BinaryOperation O2>
  using BiSame = Same<BinaryOperation, O1, O2>;
  template <UnaryOperation O1, UnaryOperation O2>
  using UnSame = Same<UnaryOperation, O1, O2>;

  template <typename T1, typename T2>
  using VecMap = std::vector<std::pair<T1, T2>>;
  using BiOp = std::function<linb::any(const linb::any&, const linb::any&)>;
  using BiMap = VecMap<std::tuple<std::type_index, std::type_index>, BiOp>;
  using UnOp = std::function<linb::any(const linb::any&)>;
  using UnMap = VecMap<std::type_index, UnOp>;

  BiMap divide_;
  BiMap multiply_;
  BiMap modulo_;
  BiMap add_;
  BiMap subtract_;
  BiMap smaller_;
  BiMap smaller_equal_;
  BiMap greater_;
  BiMap greater_equal_;
  BiMap equal_;
  BiMap not_equal_;
  UnMap bool_;
  UnMap print_;
  UnMap type_of_;
  UnMap negative_;
  UnMap positive_;

  //////////////////////////////////////////
  /// Binary
  //////////////////////////////////////////
  linb::any eval_divide(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_multiply(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_modulo(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_add(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_subtract(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_smaller(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_smaller_equal(const linb::any& lhs,
                               const linb::any& rhs) const;
  linb::any eval_greater(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_greater_equal(const linb::any& lhs,
                               const linb::any& rhs) const;
  linb::any eval_equal(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_not_equal(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_and(const linb::any& lhs, const linb::any& rhs) const;
  linb::any eval_or(const linb::any& lhs, const linb::any& rhs) const;

  //////////////////////////////////////////
  /// Unary
  //////////////////////////////////////////
  linb::any eval_not(const linb::any& rhs) const;
  linb::any eval_bool(const linb::any& rhs) const;
  linb::any eval_type_of(const linb::any& rhs) const;
  linb::any eval_print(const linb::any& rhs) const;
  linb::any eval_negative(const linb::any& rhs) const;
  linb::any eval_positive(const linb::any& rhs) const;

public:
  OperatorProvider(const bool initialize = true);

  void add(const BinaryOperation operati, std::type_index lhs,
           std::type_index rhs, BiOp operato);

  void add(const UnaryOperation operati, std::type_index lhs, UnOp operato);


  template <typename LHS, typename RHS, BinaryOperation... OPs,
            typename std::enable_if<
                (sizeof...(OPs) > 0) &&
                    none<BiSame<BinaryOperation::AND, OPs>{}...>::value &&
                    none<BiSame<BinaryOperation::OR, OPs>{}...>::value,
                bool>::type = false>
  void add() {
    std::type_index lhs(typeid(LHS));
    std::type_index rhs(typeid(RHS));

    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    (void)(  // avoid warnings of unused result
        std::initializer_list<int>{
            (add(OPs, lhs, rhs, BiHelper<LHS, RHS, OPs>()()), 0)...});
  }

  template <typename RHS, UnaryOperation... OPs,
            typename std::enable_if<
                (sizeof...(OPs) > 0) &&
                    none<UnSame<UnaryOperation::NOT, OPs>{}...>::value &&
                    none<UnSame<UnaryOperation::TYPEOF, OPs>{}...>::value &&
                    none<UnSame<UnaryOperation::PRINT, OPs>{}...>::value,
                bool>::type = false>
  void add() {
    std::type_index rhs(typeid(RHS));

    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    (void)(  // avoid warnings of unused result
        std::initializer_list<int>{
            (add(OPs, rhs, UnHelper<RHS, OPs>()()), 0)...});
  }

  template <typename LHS, typename RHS, BinaryOperation... OPs,
            typename std::enable_if<(sizeof...(OPs) > 0), bool>::type = false>
  bool has() {
    std::type_index lhs(typeid(LHS));
    std::type_index rhs(typeid(RHS));

    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    for(const auto e : std::initializer_list<bool>{has(OPs, lhs, rhs)...}) {
      if(!e) {
        return false;
      }
    }
    return true;
  }

  template <BinaryOperation... OPs,
            typename std::enable_if<(sizeof...(OPs) > 0), bool>::type = false>
  bool has(const linb::any& lhs, const linb::any& rhs) {
    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    for(const auto e : std::initializer_list<bool>{has(OPs, lhs, rhs)...}) {
      if(!e) {
        return false;
      }
    }
    return true;
  }

  template <typename RHS, UnaryOperation... OPs,
            typename std::enable_if<(sizeof...(OPs) > 0), bool>::type = false>
  bool has() {
    std::type_index rhs(typeid(RHS));

    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    for(const auto e : std::initializer_list<bool>{has(OPs, rhs)...}) {
      if(!e) {
        return false;
      }
    }
    return true;
  }

  template <UnaryOperation... OPs,
            typename std::enable_if<(sizeof...(OPs) > 0), bool>::type = false>
  bool has(const linb::any& rhs) {
    // This used the for_each_argument "trick"
    // https://isocpp.org/blog/2015/01/for-each-argument-sean-parent
    for(const auto e : std::initializer_list<bool>{has(OPs, rhs)...}) {
      if(!e) {
        return false;
      }
    }
    return true;
  }

  bool has(const BinaryOperation op, const std::type_index& lhs,
           const std::type_index& rhs) const;
  bool has(const BinaryOperation op, const linb::any& lhs,
           const linb::any& rhs) const;
  bool has(const UnaryOperation op, const std::type_index& rhs) const;
  bool has(const UnaryOperation op, const linb::any& rhs) const;

  linb::any eval(const BinaryOperation op, const linb::any& lhs,
                 const linb::any& rhs) const;
  linb::any eval(const UnaryOperation op, const linb::any& rhs) const;
};

template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::DIVIDE> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) / linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::MULTIPLY> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) * linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::MODULO> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) % linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::ADD> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) + linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::SUBTRACT> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) - linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::SMALLER> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) < linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::SMALLER_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) <= linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::GREATER> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) > linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::GREATER_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) >= linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) == linb::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::NOT_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return linb::any_cast<LHS>(lhs) != linb::any_cast<RHS>(rhs);
    };
  }
};

template <typename RHS>
struct OperatorProvider::UnHelper<RHS, OperatorProvider::UnaryOperation::BOOL> {
  auto operator()() {
    return [](auto& rhs) { return !!linb::any_cast<RHS>(rhs); };
  }
};
template <typename RHS>
struct OperatorProvider::UnHelper<RHS,
                                  OperatorProvider::UnaryOperation::NEGATIVE> {
  auto operator()() {
    return [](auto& rhs) { return -linb::any_cast<RHS>(rhs); };
  }
};
template <typename RHS>
struct OperatorProvider::UnHelper<RHS,
                                  OperatorProvider::UnaryOperation::POSITIVE> {
  auto operator()() {
    return [](auto& rhs) { return +linb::any_cast<RHS>(rhs); };
  }
};
}
}
}
#endif
