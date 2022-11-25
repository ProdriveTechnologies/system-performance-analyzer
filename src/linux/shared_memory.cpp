#include "shared_memory.h"

#include <fcntl.h>
#include <memory>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Linux
{
SharedMemory::SharedMemory(const Configuration &config)
    : config_{config}, isInitialized_{false}, sharedMemoryPtr_{nullptr}
{
}
SharedMemory::SharedMemory(caddr_t address, const Configuration &config)
    : config_{config}, isInitialized_{false}, sharedMemoryPtr_{address}
{
}
SharedMemory::~SharedMemory()
{
  munmap(sharedMemoryPtr_, config_.memorySize);
  close(sharedMemFd_);
  shm_unlink(config_.fileName.data());
}
int SharedMemory::GetModeMmap(const Mode mode)
{
  switch (mode)
  {
  case Mode::READ:
    return PROT_WRITE;
  case Mode::WRITE:
    return PROT_NONE;
  case Mode::READ_WRITE:
  default:
    return PROT_READ | PROT_WRITE;
  }
}

SharedMemory::Error SharedMemory::Allocate()
{
  // Sanity checks
  if (!isMode(Mode::WRITE))
    return Error::ILLEGAL_MODE;
  if (!isInitialized_)
  {
    auto returnVal = Init();
    if (returnVal != Error::NONE)
      return returnVal;
  }
  if (ftruncate(sharedMemFd_, config_.memorySize) < 0)
    return Error::FTRUNCATE_ERR;
  caddr_t memptr =
      (char *)mmap(NULL, config_.memorySize,
                   GetModeMmap(config_.mode), // access protections
                   MAP_SHARED,       // mapping visible to other processes
                   sharedMemFd_, 0); // offset: start at 1st byte
  if (memptr == MAP_FAILED)
    return Error::TO_MEM_ERROR; // TODO: Rename error

  sharedMemoryPtr_ = memptr;
  return Error::NONE;
}

SharedMemory::Error SharedMemory::Init()
{
  int fd = shm_open(config_.fileName.data(), /* name from smem.h */
                    GetModeShmOpen(config_.mode) |
                        O_CREAT,          /* read/write, create if needed */
                    config_.accessPerms); /* access permissions (0644) */
  if (fd < 0)
    return Error::FD_ERROR;
  isInitialized_ = true;
  sharedMemFd_ = fd;

  if (sharedMemoryPtr_ != nullptr)
  {
    caddr_t memptr =
        (char *)mmap(sharedMemoryPtr_, config_.memorySize,
                     GetModeMmap(config_.mode), // access protections
                     MAP_SHARED,       // mapping visible to other processes
                     sharedMemFd_, 0); // offset: start at 1st byte
  }

  return Error::NONE;
}

SharedMemory::Error SharedMemory::CheckSettings()
{
  // auto checkFilename(config_.fileName);
  return Error::NONE;
}
bool SharedMemory::Write(void *memoryBlock, size_t writeSize, size_t offset)
{
  if (writeSize + offset > (size_t)config_.memorySize)
  {
    return false;
  }
  auto returnVal = memcpy(sharedMemoryPtr_ + offset, memoryBlock, writeSize);

  return true;
}
bool SharedMemory::Write(caddr_t sharedMemPtr, const int memorySize,
                         void *memoryBlock, size_t writeSize, size_t offset)
{
  if (writeSize + offset > (size_t)memorySize)
  {
    return false;
  }
  auto returnVal = memcpy(sharedMemPtr + offset, memoryBlock, writeSize);

  return true;
}

std::unique_ptr<char[]> SharedMemory::Read(size_t readSize, size_t offset)
{
  if (readSize + offset > (size_t)config_.memorySize)
  {
    return NULL;
  }
  std::unique_ptr<char[]> p{new char[readSize]};
  auto returnVal = memcpy(p.get(), sharedMemoryPtr_ + offset, readSize);

  return p;
}

// int getError();

} // namespace Linux