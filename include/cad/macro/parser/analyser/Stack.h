#ifndef cad_macro_parser_analyser_Stack_h
#define cad_macro_parser_analyser_Stack_h

#include <experimental/optional>

#include <utility>
#include <vector>

namespace cad {
namespace macro {
namespace ast {
struct Variable;
namespace callable {
struct Function;
}
}
}
}

namespace cad {
namespace macro {
namespace parser {
namespace analyser {
/**
 * @brief   The Stack struct is a helper for the Analyser.
 *
 * @details This Stack is very similar to the one used by the Interpreter. The
 *          difference is that this Stack does not allocate memory for variables
 *          but only stores the names.
 */
struct Stack {
  using RV = std::reference_wrapper<const ast::Variable>;
  using RF = std::reference_wrapper<const ast::callable::Function>;

public:
  std::vector<RV> variables;
  std::vector<RF> functions;
  Stack* parent = nullptr;

public:
  /**
   * @brief  Determine if it has var.
   *
   * @param  name  The name of the variable to check
   *
   * @return true if has var, false otherwise.
   */
  bool has_var(const std::string& name) const;
  /**
   * @return an optional pair where the first instance is the variable that was
   *         first declared and the second the variable that was declared last
   */
  std::experimental::optional<std::pair<RV, RV>> has_double_var() const;
  /**
   * @return an optional pair where the first instance is the function that was
   *         first declared and the second the function that was declared last
   */
  std::experimental::optional<std::pair<RF, RF>> has_double_fun() const;
};
}
}
}
}
#endif
