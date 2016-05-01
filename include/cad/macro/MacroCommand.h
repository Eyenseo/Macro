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
/**
 * @brief  The MacroCommand can be used to execute a Macro with Arguments
 */
class MacroCommand : public core::command::Command {
  std::weak_ptr<interpreter::OperatorProvider> op_provider_;
  std::weak_ptr<CommandProvider> command_provider_;

public:
  enum class E { MISSING_PROVIDER };

  /**
   * @brief  Ctor
   *
   * @param  op_provider       The OperatorProvider
   * @param  command_provider  The CommandProvider
   */
  MacroCommand(std::weak_ptr<interpreter::OperatorProvider> op_provider,
               std::weak_ptr<CommandProvider> command_provider);

  /**
   * @brief  Executes the Command
   *
   * @param  args    The arguments for the Command: 'Macro' (string) as the
   *                 macro to be executed 'Arguments'(Arguments) the arguments
   *                 the macro will be executed with, 'Macroname'(string) the
   *                 name of the macro or file it is from and 'Output'
   *                 (std::reference_wrapper<std::ostream>) as the ouptut that
   *                 will be used for the print operator.
   *
   * @return can be anything
   *
   * @throws Exc<E,  E::MISSING_PROVIDER>
   */
  linb::any execute(Arguments args) override;

  std::shared_ptr<Command> clone() const override;
};
}
}
#endif
