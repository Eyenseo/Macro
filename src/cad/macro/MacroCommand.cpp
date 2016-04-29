#include "cad/macro/MacroCommand.h"

#include <cad/macro/interpreter/Interpreter.h>


#include <cad/core/command/argument/Arguments.h>

namespace cad {
namespace macro {
namespace {
using Command = cad::core::command::Command;
}

MacroCommand::MacroCommand(
    std::weak_ptr<interpreter::OperatorProvider> op_provider,
    std::weak_ptr<CommandProvider> command_provider)
    : Command("eval_macro", command_provider)
    , op_provider_(op_provider)
    , command_provider_(command_provider) {
  set_description("MacroCommand");

  Arguments args;
  args.add("Macro", "Macro to execute.", std::string());
  args.add("Macroname", "Name of the macro.", std::string(), true);
  args.add("Output", "Output stream.",
           std::reference_wrapper<std::ostream>(std::cout), true);
  args.add("Arguments", "Arguments for the macro", Arguments(), true);
  set_arguments(args);

  set_modifying(false);
  set_undoable(false);
}

linb::any MacroCommand::execute(Arguments args) {
  // TODO Single History step
  // at the moment each command that gets called by the makro will lead to a new
  // step in the History
  auto com_pro = command_provider_.lock();
  auto op_pro = op_provider_.lock();

  if(!com_pro) {
    Exc<E, E::MISSING_PROVIDER> e(__FILE__, __LINE__, "Missing Provider");
    e << "The macro command needs a CommandProvider but the one given was a "
         "nullptr.";
    throw e;
  } else if(!op_pro) {
    Exc<E, E::MISSING_PROVIDER> e(__FILE__, __LINE__, "Missing Provider");
    e << "The macro command needs a OperatorProvider but the one given was a "
         "nullptr.";
    throw e;
  }

  interpreter::Interpreter inter = [&] {
    if(auto out = args.get<std::reference_wrapper<std::ostream>>("Output")) {
      return interpreter::Interpreter(com_pro, op_pro, *out);
    } else {
      return interpreter::Interpreter(com_pro, op_pro);
    }
  }();

  return [&] {
    if(auto file = args.get<std::string>("Macroname")) {
      return inter.interpret(*args.get<std::string>("Makro"),
                             *args.get<Arguments>("Arguments"), get_scope(),
                             *file);
    } else {
      return inter.interpret(*args.get<std::string>("Makro"),
                             *args.get<Arguments>("Arguments"), get_scope());
    }
  }();
}

std::shared_ptr<Command> MacroCommand::clone() const {
  return std::dynamic_pointer_cast<Command>(
      std::make_shared<MacroCommand>(*static_cast<const MacroCommand*>(this)));
}
}
}
