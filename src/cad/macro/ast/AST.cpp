#include "cad/macro/ast/AST.h"

#include "cad/macro/IndentStream.h"

namespace cad {
namespace macro {
namespace ast {
AST::AST(parser::Token token)
    : token_(std::move(token)) {
}
}
}
}
