#pragma once

#include "src/exports/export_struct.h"

namespace Linux
{
class CPathParserBase
{
public:
  virtual void Initialize() = 0;

  virtual bool InitializeRuntime(const std::string &replacement = "") = 0;
  virtual bool ParseMeasurement(const PlatformConfig::SDatafields &datafield,
                                const std::string &path,
                                const std::string &replacement = "") = 0;
  Measurements::SMeasuredItem GetMeasurement() const { return item_; }
  std::string GetErrorMsg() const { return errorMsg_; }

protected:
  Measurements::SMeasuredItem item_;
  std::string errorMsg_;

private:
};
} // namespace Linux