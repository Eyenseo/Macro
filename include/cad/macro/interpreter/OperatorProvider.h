#ifndef cad_macro_interpreter_OperatorProvider_h
#define cad_macro_interpreter_OperatorProvider_h

#include <functional>

#include <core/any.hpp>

namespace cad {
namespace macro {
namespace interpreter {
class OperatorProvider {
public:
  enum class UnaryOperation { NOT, BOOL };
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
  enum class E { MISSING_OPERATOR, TODO };

protected:
  template <typename LHS, typename RHS, BinaryOperation op>
  struct BiHelper;
  template <typename RHS, UnaryOperation op>
  struct UnHelper;

  template <bool...>
  struct bool_pack;
  template <bool... v>
  using all = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;

  template <typename T, T O1, T O2>
  struct Same
      : std::conditional<O1 == O2, std::true_type, std::false_type>::type {};
  template <BinaryOperation O1, BinaryOperation O2>
  using BiSame = Same<BinaryOperation, O1, O2>;
  template <UnaryOperation O1, UnaryOperation O2>
  using UnSame = Same<UnaryOperation, O1, O2>;

  template <typename T1, typename T2>
  using VecMap = std::vector<std::pair<T1, T2>>;
  using BiOp =
      std::function<::core::any(const ::core::any&, const ::core::any&)>;
  using BiMap = VecMap<std::tuple<std::type_index, std::type_index>, BiOp>;
  using UnOp = std::function<::core::any(const ::core::any&)>;
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

  //////////////////////////////////////////
  /// Binary
  //////////////////////////////////////////
  ::core::any eval_divide(const ::core::any& lhs, const ::core::any& rhs) const;
  ::core::any eval_multiply(const ::core::any& lhs,
                            const ::core::any& rhs) const;
  ::core::any eval_modulo(const ::core::any& lhs, const ::core::any& rhs) const;
  ::core::any eval_add(const ::core::any& lhs, const ::core::any& rhs) const;
  ::core::any eval_subtract(const ::core::any& lhs,
                            const ::core::any& rhs) const;
  ::core::any eval_smaller(const ::core::any& lhs,
                           const ::core::any& rhs) const;
  ::core::any eval_smaller_equal(const ::core::any& lhs,
                                 const ::core::any& rhs) const;
  ::core::any eval_greater(const ::core::any& lhs,
                           const ::core::any& rhs) const;
  ::core::any eval_greater_equal(const ::core::any& lhs,
                                 const ::core::any& rhs) const;
  ::core::any eval_equal(const ::core::any& lhs, const ::core::any& rhs) const;
  ::core::any eval_not_equal(const ::core::any& lhs,
                             const ::core::any& rhs) const;
  ::core::any eval_and(const ::core::any& lhs, const ::core::any& rhs) const;
  ::core::any eval_or(const ::core::any& lhs, const ::core::any& rhs) const;

  //////////////////////////////////////////
  /// Unary
  //////////////////////////////////////////
  ::core::any eval_not(const ::core::any& rhs) const;
  ::core::any eval_bool(const ::core::any& rhs) const;

public:
  OperatorProvider(const bool initialize = true);

  void add(const BinaryOperation operati, std::type_index lhs,
           std::type_index rhs, BiOp operato);

  void add(const UnaryOperation operati, std::type_index lhs, UnOp operato);


  template <typename LHS, typename RHS, BinaryOperation... OPs,
            typename std::enable_if<
                (sizeof...(OPs) > 0) &&
                    !all<BiSame<BinaryOperation::AND, OPs>{}...>::value &&
                    !all<BiSame<BinaryOperation::OR, OPs>{}...>::value,
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
                    !all<UnSame<UnaryOperation::NOT, OPs>{}...>::value,
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
  bool has(const ::core::any& lhs, const ::core::any& rhs) {
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
  bool has(const ::core::any& rhs) {
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
  bool has(const BinaryOperation op, const ::core::any& lhs,
           const ::core::any& rhs) const;
  bool has(const UnaryOperation op, const std::type_index& rhs) const;
  bool has(const UnaryOperation op, const ::core::any& rhs) const;

  ::core::any eval(const BinaryOperation op, const ::core::any& lhs,
                   const ::core::any& rhs) const;
  ::core::any eval(const UnaryOperation op, const ::core::any& rhs) const;
};

template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::DIVIDE> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) / ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::MULTIPLY> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) * ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::MODULO> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) % ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::ADD> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) + ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::SUBTRACT> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) - ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::SMALLER> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) < ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::SMALLER_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) <= ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::GREATER> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) > ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::GREATER_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) >= ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<LHS, RHS,
                                  OperatorProvider::BinaryOperation::EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) == ::core::any_cast<RHS>(rhs);
    };
  }
};
template <typename LHS, typename RHS>
struct OperatorProvider::BiHelper<
    LHS, RHS, OperatorProvider::BinaryOperation::NOT_EQUAL> {
  auto operator()() {
    return [](auto& lhs, auto& rhs) {
      return ::core::any_cast<LHS>(lhs) != ::core::any_cast<RHS>(rhs);
    };
  }
};

template <typename RHS>
struct OperatorProvider::UnHelper<RHS, OperatorProvider::UnaryOperation::BOOL> {
  auto operator()() {
    return [](auto& rhs) { return !!::core::any_cast<RHS>(rhs); };
  }
};
}
}
}
#endif
