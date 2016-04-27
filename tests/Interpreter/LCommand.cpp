#include "LCommand.h"

namespace {
using Command = cad::core::command::Command;
}

LCommand::LCommand(std::string call_name,
                   std::weak_ptr<CommandProvider> command_provider,
                   std::function<linb::any(Arguments)> fun)
    : LCommand(std::move(call_name), std::move(command_provider),
               std::move(fun), Arguments()) {
}

LCommand::LCommand(std::string call_name,
                   std::weak_ptr<CommandProvider> command_provider,
                   std::function<linb::any(Arguments)> fun, Arguments args)
    : Command(std::move(call_name), command_provider)
    , fun_(std::move(fun)) {
  set_arguments(std::move(args));
}

linb::any LCommand::execute(Arguments args) {
  auto ret = fun_(args);
  active_signal().emit(false);
  return ret;
}

std::shared_ptr<Command> LCommand::clone() const {
  return std::dynamic_pointer_cast<Command>(
      std::make_shared<LCommand>(*static_cast<const LCommand*>(this)));
}
