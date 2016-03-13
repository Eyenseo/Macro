#include "cad/macro/ast/Define.h"

namespace cad {
namespace macro {
namespace ast {
Define::Define(parser::Token token)
    : AST(std::move(token)) {
}
}
}
}
