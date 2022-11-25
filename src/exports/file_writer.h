#pragma once

#include <fstream>

/**
 * @brief Wrapper around a C++ file object. Adds the functionality of writing
 * data row by row explicitly
 */
class FileWriter
{
public:
  FileWriter(const std::string &fileName);

  bool AddRow(const std::string &text);
  bool AddRow(const std::string &text, const bool addEmptyString);
  void Close() { file_.close(); }

private:
  std::ofstream file_;
};