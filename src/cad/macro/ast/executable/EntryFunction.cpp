#include "cad/macro/ast/executable/EntryFunction.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
EntryFunction::EntryFunction(parser::Token token)
    : Function(std::move(token)) {
}
}
}
}
}
