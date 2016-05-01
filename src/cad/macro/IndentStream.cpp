#include <cad/macro/IndentStream.h>

namespace cad {
namespace macro {
IndentStream::IndentStream(std::ostream& os, const long step,
                           const long indention)
    : buf(os.rdbuf(), step, indention) {
  rdbuf(&buf);
}

IndentStream& IndentStream::indent() {
  buf.indent();
  return *this;
}
IndentStream& IndentStream::dedent() {
  buf.dedent();
  return *this;
}
}
}
