#ifndef cad_macro_parser_analyser_Stack_h
#define cad_macro_parser_analyser_Stack_h

#include <experimental/optional>

#include <utility>
#include <vector>

namespace cad {
namespace macro {
namespace ast {
class Variable;
namespace callable {
class Function;
}
}
}
}

namespace cad {
namespace macro {
namespace parser {
namespace analyser {
struct Stack {
  using RV = std::reference_wrapper<const ast::Variable>;

  // FIXME
  template <typename T>
  struct myPair {  // optional doesn't like std::pair or std::tuple ...
    std::reference_wrapper<const T> first;
    std::reference_wrapper<const T> second;
  };

public:
  std::vector<std::reference_wrapper<const ast::Variable>> variables;
  std::vector<std::reference_wrapper<const ast::callable::Function>> functions;
  Stack* parent = nullptr;

public:
  std::experimental::optional<myPair<ast::Variable>> has_double_var() const;
  bool has_var(const std::string& name) const;
  std::experimental::optional<myPair<ast::callable::Function>>
  has_double_fun() const;
};
}
}
}
}
#endif
