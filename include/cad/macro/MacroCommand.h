#ifndef cad_macro_MacroCommand_h
#define cad_macro_MacroCommand_h

#include <cad/core/command/Command.h>

namespace cad {
namespace macro {
namespace interpreter {
class OperatorProvider;
}
}
}

namespace cad {
namespace macro {
class MacroCommand : public core::command::Command {
  std::weak_ptr<interpreter::OperatorProvider> op_provider_;
  std::weak_ptr<CommandProvider> command_provider_;

public:
  enum class E { MISSING_PROVIDER };

  explicit MacroCommand(
      std::weak_ptr<interpreter::OperatorProvider> op_provider,
      std::weak_ptr<CommandProvider> command_provider);

  linb::any execute(Arguments args) override;

  std::shared_ptr<Command> clone() const override;
};
}
}
#endif
