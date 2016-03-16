#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
Variable::Variable(parser::Token token)
    : AST(std::move(token)) {
}
}
}
}
