#include "cad/macro/MacroInitializer.h"

namespace {
using ModuleProxy = p3::common::module_system::ModuleProxy;
using ModuleLoader = p3::common::module_system::ModuleLoader;
using PProvider = p3::common::module_system::ProviderProvider;
using ThreadManager = p3::common::module_system::ThreadManager;

using Initializer = cad::macro::MacroInitializer;
}

/**
 * @brief Retrieve the loader version we're going to expect
 */
extern "C" P3_ABI(cad_gui) unsigned int get_loader_target_version() {
  return 4;
}

/**
 * @brief Register the module
 *
 * @param provider  Provider that provides BaseProvider
 * @param manager   Thread manager to run asynchronous tasks with
 */
extern "C" P3_ABI(cad_gui) ModuleProxy::Status
    initialise_module(std::weak_ptr<ModuleLoader> loader,
                      std::weak_ptr<PProvider> provider,
                      std::weak_ptr<ThreadManager> manager) {
  static Initializer ini(loader, provider, manager);
  ini.initialize();

  return ini.status();
}
