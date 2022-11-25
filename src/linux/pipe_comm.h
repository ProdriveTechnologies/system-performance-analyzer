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

  void Write(std::string message);
  void Write(void *message, size_t bytes);
  std::string Read();
  void Read(void *message, size_t bytes);

private:
  int parentReadPipe_[2];
  int parentWritePipe_[2];
  bool isParent_;

  static constexpr int READ = 0;
  static constexpr int WRITE = 1;
  static constexpr int READ_BUFFER_SIZE = 1024;
};

} // namespace Linux