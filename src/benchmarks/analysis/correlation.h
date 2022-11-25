#pragma once

#include <vector>

namespace Measurements
{
class CCorrelation
{
public:
  double GetCorrelationCoefficient(const std::vector<double> &u,
                                   const std::vector<double> &v);

private:
};

} // namespace Measurements