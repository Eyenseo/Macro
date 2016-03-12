#include <cad/macro/IndentStream.h>

namespace cad {
namespace macro {
IndentStream::IndentStream(std::ostream& os)
    : buf(os.rdbuf()) {
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
