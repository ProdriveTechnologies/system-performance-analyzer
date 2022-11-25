#include "pipe_comm.h"

#include <memory>
#include <stdexcept>
#include <string.h>
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
  close(parentReadPipe_[READ]);
  close(parentWritePipe_[WRITE]);
}

void PipeCommunicator::SetParent()
{
  isParent_ = true;
  close(parentReadPipe_[WRITE]);
  close(parentWritePipe_[READ]);
}

void PipeCommunicator::Write(std::string message)
{
  Write(static_cast<void *>(&message), message.size());
}
void PipeCommunicator::Write(void *message, size_t bytes)
{
  int fd = isParent_ ? parentWritePipe_[WRITE] : parentReadPipe_[WRITE];
  int writtenBytes = write(fd, message, bytes + 1);
  if (writtenBytes < 0)
  {
    std::string errorStr(strerror(errno));
    throw std::runtime_error("PipeCommunicator: Couldn't write!" + errorStr);
  }
}

std::string PipeCommunicator::Read()
{
  std::unique_ptr<char[]> charBuff{new char[READ_BUFFER_SIZE]};
  int fd = isParent_ ? parentReadPipe_[WRITE] : parentWritePipe_[WRITE];
  int readBytes = read(fd, charBuff.get(), READ_BUFFER_SIZE);
  if (readBytes != READ_BUFFER_SIZE && readBytes >= 0)
  {
    charBuff[readBytes] = '\0';
  }
  else
  {
    std::runtime_error("PipeCommunicator: Read buffer exceeded!");
  }

  return charBuff.get();
}

} // namespace Linux