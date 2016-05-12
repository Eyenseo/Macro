#include "cad/macro/MacroInitializer.h"

#include "cad/macro/MacroCommand.h"
#include "cad/macro/interpreter/OperatorProvider.h"

#include <cad/core/command/CommandProvider.h>
#include <cad/core/command/MenuFilter.h>

#include <p3/common/module_system/SignalProvider.h>

namespace cad {
namespace macro {
namespace {
using OperatorProvider = interpreter::OperatorProvider;

using SignalProvider = p3::common::module_system::SignalProvider;

using CommandProvider = cad::core::command::CommandProvider;
using MenuFilter = cad::core::command::MenuFilter;

using namespace macro_initializer;
}

MacroInitializer::MacroInitializer(const std::weak_ptr<ModuleLoader>& loader,
                                   const std::weak_ptr<PProvider>& provider,
                                   const std::weak_ptr<ThreadManager>& manager)
    : cad::core::CoreInitializerBase<STEPS>(loader, provider, manager) {
}

void MacroInitializer::initialize() {
  try {
    auto com_pro = obtain_provider<CommandProvider>();
    auto op_pro =
        add_get_provider<STEPS::OPERATOR_PROVIDER, OperatorProvider>();

    add_command<STEPS::MACRO_COMMAND>()
        .name("ExecuteMakro")
        .scope("")
        .weight(1000)
        .add<MacroCommand>(op_pro, com_pro);

  } catch(std::exception& e) {
    emit_exception(e);
  }
}
}
}
