#include "cad/macro/ast/loop/DoWhile.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
DoWhile::DoWhile(parser::Token token)
    : While(std::move(token)) {
}
}
}
}
}
