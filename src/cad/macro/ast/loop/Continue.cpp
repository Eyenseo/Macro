#include "cad/macro/ast/loop/Continue.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
Continue::Continue() {
}

Continue::Continue(parser::Token token)
    : AST(std::move(token)) {
}
}
}
}
}
