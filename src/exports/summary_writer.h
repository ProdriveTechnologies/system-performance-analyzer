#pragma once

#include <cmath>
#include <string>

/**
 * @brief Wrapper around a C++ file object. Adds the functionality of writing
 * data row by row explicitly
 */
class SummaryWriter
{
public:
  SummaryWriter() = default;
  ~SummaryWriter() = default;
  static void PrintSection(const std::string_view& section);
  static void PrintSubSection(const std::string_view& subsection);
  static void PrintTitle(const std::string_view& title);
  static void PrintValue(const std::string_view& text, const std::string& value);
  static void PrintValue(const std::string_view& text, const std::string_view& value);
  template <typename T>
  static void PrintValue(const std::string_view& text, const T& value);
  template <typename T>
  static void PrintValue(const std::string_view& text, const T& value, const std::string& suffix);

  static void PrintRow(const std::string_view& text = "");

private:
  struct Line
  {
    char character;
    unsigned int size;
  };
  static constexpr Line sectionLine_{ '=', 92 };
  static constexpr Line subsectionLine_{ '-', 48 };
  static constexpr std::string_view rowCenter_ = " : ";
  static constexpr int tabRowLength_ = 32;
  static constexpr int tabSize_ = 8;
  static inline std::string GetTabs(const size_t nonTabChars)
  {
    std::string tabs;
    int tabsNr = std::ceil((tabRowLength_ - static_cast<int>(nonTabChars)) / static_cast<double>(tabSize_));
    for (int i = 0; i < tabsNr; ++i)
      tabs += '\t';
    return tabs;
  }
  static inline void PrintLine(const Line& line)
  {
    std::string lineStr;
    for (unsigned i = 0; i < line.size; i++)
      lineStr += line.character;
    PrintRow(lineStr);
  }
};

template <typename T>
void SummaryWriter::PrintValue(const std::string_view& text, const T& value)
{
  PrintValue(text, std::to_string(value));
}

template <typename T>
void SummaryWriter::PrintValue(const std::string_view& text, const T& value, const std::string& suffix)
{
  PrintValue(text, std::to_string(value) + " " + suffix);
}