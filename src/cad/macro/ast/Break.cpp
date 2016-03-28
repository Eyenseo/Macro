#include "cad/macro/ast/Break.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
Break::Break() {
}

Break::Break(parser::Token token)
    : AST(std::move(token)) {
}
}
}
}
