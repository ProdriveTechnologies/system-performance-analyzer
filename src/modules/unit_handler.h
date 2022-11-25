#pragma once

#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include <iostream> // TODO: Remove, only used for logging

namespace Core
{
class UnitHandler
{
public:
  UnitHandler() = default;
  template <typename ModuleType> pid_t AddUnit(Module::Config moduleConfig)
  {
    pid_t applicationPid = fork();
    if (applicationPid < 0)
    {
      std::string response{strerror(errno)};
      response = "Fork failed! Reason: " + response;
      throw std::runtime_error(response);
    }
    else if (applicationPid == 0)
    {
      // Child process
      ModuleType moduleObj{moduleConfig};
      moduleObj.Run();
      std::cout << "Child is terminating" << std::endl;
      // Stop the child when it returns (and close pipes accordingly)
      // close(moduleConfig.readPipeParent);
      // close(moduleConfig.writePipeParent);
      exit(EXIT_SUCCESS);
    }
    else
    {
      // Parent process
      return applicationPid;
    }
  }

private:
};

} // namespace Core