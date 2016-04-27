#include "cad/macro/parser/analyser/Stack.h"

#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Function.h"

#include <algorithm>

namespace cad {
namespace macro {
namespace parser {
namespace analyser {

std::experimental::optional<Stack::myPair<ast::Variable>>
Stack::has_double_var() const {
  if(variables.size() > 1) {
    const auto& back = variables.back().get();
    auto it = std::find_if(variables.begin(), variables.end() - 1,
                           [&back](const auto& var) {
                             return var.get().token.token == back.token.token;
                           });

    if(variables.end() - 1 != it) {
      return {{*it, back}};
    }
  }
  return {};
}

bool Stack::has_var(const std::string& name) const {
  if(variables.end() !=
     std::find_if(variables.begin(), variables.end(), [&name](const auto& var) {
       return var.get().token.token == name;
     })) {
    return true;
  } else if(parent) {
    return parent->has_var(name);
  }
  return false;
}

std::experimental::optional<Stack::myPair<ast::callable::Function>>
Stack::has_double_fun() const {
  if(functions.size() > 1) {
    auto& back = functions.back().get();
    auto it = std::find_if(
        functions.begin(), functions.end() - 1, [&back](const auto& fun) {
          if(fun.get().token.token == back.token.token &&
             fun.get().parameter.size() == back.parameter.size()) {
            // Parameter
            for(const auto& fp : fun.get().parameter) {
              bool found = false;
              for(const auto& cp : back.parameter) {
                if(fp.token.token == cp.token.token) {
                  found = true;
                  break;
                }
              }
              if(!found) {
                return false;
              }
            }
          } else {
            return false;
          }
          return true;
        });

    if(functions.end() - 1 != it) {
      return {{*it, back}};
    }
  }
  return {};
}
}
}
}
}
