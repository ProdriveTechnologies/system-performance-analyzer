#include "summary_writer.h"

#include <iostream>

void SummaryWriter::PrintValue(const std::string_view &text,
                               const std::string &value)
{
  std::string row{text};
  row += GetTabs(text.size());
  row += rowCenter_;
  row += value;
  PrintRow(row);
}

void SummaryWriter::PrintValue(const std::string_view &text,
                               const std::string_view &value)
{
  PrintValue(text, std::string(value));
}

void SummaryWriter::PrintSection(const std::string_view &section)
{
  PrintRow(section);
  PrintLine(sectionLine_);
}

void SummaryWriter::PrintSubSection(const std::string_view &subsection)
{
  PrintRow(subsection);
  PrintLine(subsectionLine_);
}

/**
 * @brief
 *
 * @param title
 */
void SummaryWriter::PrintTitle(const std::string_view &title)
{
  PrintRow(title);
  PrintRow();
}

/**
 * @brief Prints a row to the std::out
 *
 * @param text the text to print on the row
 */
void SummaryWriter::PrintRow(const std::string_view &text)
{
  std::cout << text << std::endl;
}