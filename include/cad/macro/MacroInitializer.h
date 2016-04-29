#ifndef cad_macro_MacroInitializer_h
#define cad_macro_MacroInitializer_h

#include <cad/core/CoreInitializerBase.h>

namespace cad {
namespace macro {
namespace macro_initializer {
enum class STEPS { OPERATOR_PROVIDER, MACRO_COMMAND, SIZE };
}

/**
 * @brief Initializer for the Macro module
 */
class MacroInitializer
    : public cad::core::CoreInitializerBase<macro_initializer::STEPS> {
public:
  /**
   * @brief Ctor
   *
   * @param loader    ModuleLoader
   * @param provider  ProviderProvider
   * @param manager   ThreadManager
   */
  MacroInitializer(const std::weak_ptr<ModuleLoader>& loader,
                   const std::weak_ptr<PProvider>& provider,
                   const std::weak_ptr<ThreadManager>& manager);

  /**
   * @brief Initializes the Macro module
   */
  void initialize();
};
}
}
#endif
