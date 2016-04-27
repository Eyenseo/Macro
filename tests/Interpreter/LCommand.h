#ifndef LCommand_h
#define LCommand_h

#include <cad/core/command/Command.h>
#include <cad/core/command/argument/Arguments.h>

/**
 * @brief LCommand can be used to execute a lambda as a Command
 */
class LCommand : public cad::core::command::Command {
  using CommandProvider = cad::core::command::CommandProvider;

  std::function<linb::any(Arguments)> fun_;

public:
  /**
   * @brief Constructor
   *
   * @param command_provider  CommandProvider to request other Command objects
   *                          from to achieve this commands purpose
   * @param fun               Function to be executed
   */
  LCommand(std::string call_name,
           std::weak_ptr<CommandProvider> command_provider,
           std::function<linb::any(Arguments)> fun);
  /**
   * @brief Constructor
   *
   * @param command_provider  CommandProvider to request other Command objects
   *                          from to achieve this commands purpose
   * @param fun               Function to be executed
   * @param args              Arguments the function expects to obtain
   */
  LCommand(std::string call_name,
           std::weak_ptr<CommandProvider> command_provider,
           std::function<linb::any(Arguments)> fun, Arguments args);

  /**
   * @brief Function to execute the function
   *
   * @param args Arguments to be given to the function
   *
   * @return Retrun values from the function
   */
  linb::any execute(Arguments args) override;
  /**
   * @brief Clones this Command
   *
   * @return clone of this Command
   */
  std::shared_ptr<Command> clone() const override;
};
#endif
