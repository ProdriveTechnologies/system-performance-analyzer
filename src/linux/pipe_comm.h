#pragma once

#include <string>

namespace Linux
{
class PipeCommunicator
{
public:
  PipeCommunicator();
  void SetChild();
  void SetParent();

  void Write(const std::string &message);
  void Write(void *message, const size_t bytes);
  std::string Read();
  size_t Read(void *message, const size_t bytes);
  std::string ReadUntil(const size_t bytes);

private:
  int parentReadPipe_[2];
  int parentWritePipe_[2];
  bool isParent_;

  static constexpr int READ = 0;
  static constexpr int WRITE = 1;
  static constexpr size_t READ_BUFFER_SIZE = 1024;
};

} // namespace Linux