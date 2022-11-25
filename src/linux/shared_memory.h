#pragma once

#include <fcntl.h> /* For O_* constants */
#include <memory>
#include <string>

namespace Linux
{
class SharedMemory
{
public:
  enum class Mode : int
  {
    READ = O_RDONLY,
    WRITE = O_WRONLY,
    READ_WRITE = O_RDWR
  };
  int GetModeShmOpen(const Mode mode)
  {
    switch (mode)
    {
    case Mode::READ:
      return O_RDONLY;
    case Mode::WRITE:
      return O_WRONLY;
    case Mode::READ_WRITE:
      return O_RDWR;
    }
  }
  int GetModeMmap(const Mode mode);

  enum class Error
  {
    NONE,
    ILLEGAL_MODE,
    FD_ERROR,
    TO_MEM_ERROR,
    FTRUNCATE_ERR
  };
  struct Configuration
  {
    std::string fileName;
    Mode mode = Mode::READ_WRITE;
    bool autoCreateObject = true;
    int accessPerms =
        S_IRUSR | S_IWUSR | S_IRGRP |
        S_IROTH; // Access permissions 0644 for the created shared memory file
    off_t memorySize;
  };

  SharedMemory(const Configuration &config);
  SharedMemory(const caddr_t address, const Configuration &config);
  ~SharedMemory();

  Error Init();
  Error Allocate();
  bool Write(void *memoryBlock, size_t writeSize, size_t offset = 0);
  static bool Write(caddr_t sharedMemPtr, const int memorySize,
                    void *memoryBlock, size_t writeSize, size_t offset = 0);
  std::unique_ptr<char[]> Read(size_t readSize, size_t offset = 0);

  Configuration GetConfig() const { return config_; }
  caddr_t GetAddress() const { return sharedMemoryPtr_; }

private:
  const Configuration config_;
  int sharedMemFd_;
  bool isInitialized_;
  caddr_t sharedMemoryPtr_; // Pointer to the shared memory block

  Error CheckSettings();

  bool isMode(const Mode wantedMode)
  {
    // READ_WRITE mode is always the wanted mode
    return (wantedMode == config_.mode) || (config_.mode == Mode::READ_WRITE);
  }
};

} // namespace Linux