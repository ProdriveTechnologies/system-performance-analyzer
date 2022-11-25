#include "file_writer.h"

FileWriter::FileWriter(const std::string &fileName) : file_{}
{
  file_.open(fileName.c_str(), std::ios::trunc | std::ios::out);
}

/**
 * @brief adds a row in the file
 *
 * @return true no errors occured
 * @return false could not add the text in the file, file corrupt
 */
bool FileWriter::AddRow(const std::string &text)
{
  if (file_.is_open())
  {
    file_ << text << "\n";
    return true;
  }
  return false;
}

/**
 * @brief adds a row in the file, except if "addEmptyString" is false and "text"
 * is empty
 *
 * @return true no errors occured
 * @return false could not add the text in the file, file corrupt
 */
bool FileWriter::AddRow(const std::string &text, const bool addEmptyString)
{
  auto checkEmpty = [this](const std::string &text) {
    return text.empty() ? true : AddRow(text);
  };
  return addEmptyString ? AddRow(text) : checkEmpty(text);
}