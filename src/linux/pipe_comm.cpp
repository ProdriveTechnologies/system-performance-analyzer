#include "pipe_comm.h"

#include <memory>
#include <stdexcept>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Linux
{
PipeCommunicator::PipeCommunicator()
{
  int status = pipe(parentReadPipe_);
  int status2 = pipe(parentWritePipe_);
  if (status < 0 || status2 < 0)
  {
    throw std::runtime_error("Couldn't create pipe!");
  }
}

void PipeCommunicator::SetChild()
{
  isParent_ = false;
  close(parentWritePipe_[WRITE]);
  close(parentReadPipe_[READ]);
}

void PipeCommunicator::SetParent()
{
  isParent_ = true;
  close(parentReadPipe_[WRITE]);
  close(parentWritePipe_[READ]);
}

void PipeCommunicator::Write(const std::string &message)
{
  // Write(static_cast<void *>(&message), message.size());

  int fd = isParent_ ? parentWritePipe_[WRITE] : parentReadPipe_[WRITE];
  int writtenBytes = write(fd, message.c_str(), message.size());
  if (writtenBytes < 0)
  {
    std::string errorStr(strerror(errno));
    throw std::runtime_error("PipeCommunicator: Couldn't write!" + errorStr);
  }
}

void PipeCommunicator::Write(void *message, const size_t bytes)
{
  int fd = isParent_ ? parentWritePipe_[WRITE] : parentReadPipe_[WRITE];
  int writtenBytes = write(fd, message, bytes);
  if (writtenBytes < 0)
  {
    std::string errorStr(strerror(errno));
    throw std::runtime_error("PipeCommunicator: Couldn't write!" + errorStr);
  }
}

std::string PipeCommunicator::Read()
{
  char charBuff[READ_BUFFER_SIZE];
  int fd = isParent_ ? parentReadPipe_[READ] : parentWritePipe_[READ];
  ssize_t readBytes = read(fd, charBuff, READ_BUFFER_SIZE);
  if (readBytes != READ_BUFFER_SIZE && readBytes >= 0)
  {
    charBuff[readBytes] = '\0';
  }
  else
  {
    std::runtime_error("PipeCommunicator: Read buffer exceeded!");
  }

  return charBuff;
}

/**
 * @brief Reads until X bytes are read or when the read function returns (can be
 * less than X bytes)
 */
size_t PipeCommunicator::Read(void *message, const size_t bytes)
{
  int fd = isParent_ ? parentReadPipe_[READ] : parentWritePipe_[READ];
  size_t readBytes = read(fd, message, bytes);
  if (readBytes != READ_BUFFER_SIZE)
    return readBytes;
  throw std::runtime_error("PipeCommunicator: Read buffer exceeded!");
}

/**
 * @brief Reads until X bytes are read, will wait until it does
 */
std::string PipeCommunicator::ReadUntil(const size_t bytes)
{
  size_t readBytes;

  char messageData[bytes + 1];
  while (readBytes != bytes)
  {
    size_t returnRead = Read(&messageData[readBytes], bytes - readBytes);
    if (returnRead > 0)
      readBytes += returnRead;
  }
  messageData[bytes] = '\0';
  return messageData;
}

} // namespace Linux