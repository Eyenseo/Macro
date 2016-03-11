#include "cad/macro/MacroInitializer.h"

#include <cad/core/command/CommandProvider.h>

#include <p3/common/module_system/SignalProvider.h>

namespace cad {
namespace macro {
namespace {
using SignalProvider = p3::common::module_system::SignalProvider;

using namespace macro_initializer;
}

MacroInitializer::MacroInitializer(const std::weak_ptr<ModuleLoader>& loader,
                                   const std::weak_ptr<PProvider>& provider,
                                   const std::weak_ptr<ThreadManager>& manager)
    : cad::core::CoreInitializerBase<STEPS>(loader, provider, manager) {
}

void MacroInitializer::initialize() {
  try {
    auto si_pro = obtain_provider<SignalProvider>();
    auto com_pro = obtain_provider<CommandProvider>();
  } catch(std::exception& e) {
    emit_exception(e);
  }
}
}
}
