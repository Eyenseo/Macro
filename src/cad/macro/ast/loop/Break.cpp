#include "cad/macro/ast/loop/Break.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
Break::Break() {
}

Break::Break(parser::Token token)
    : AST(std::move(token)) {
}
}
}
}
}
