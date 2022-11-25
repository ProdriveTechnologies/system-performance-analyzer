#pragma once

#include "src/modules/base_class.h"

namespace Module
{
class FileStream : public Base
{
public:
  struct JsonVars
  {
    static constexpr char FileName[] = "FILE";
    static constexpr char BlockSize[] = "BLOCK_SIZE";
    static constexpr char SharedMemorySize[] = "SHARED_MEM_TOTAL_SIZE";
  };

  // using Base::Base; // For the constructor
  FileStream(Config &moduleConfig);
  void Init();
  bool CheckCustomVars();
  void Run();
  void Loop();
  void SetSharedMemory();

private:
};

} // namespace Module