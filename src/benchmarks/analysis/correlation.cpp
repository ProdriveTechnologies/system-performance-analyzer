#include "correlation.h"

#if BOOST_VERSION / 100 >= 1072
#include <boost/math/statistics/bivariate_statistics.hpp>
#else
#include <boost/math/tools/bivariate_statistics.hpp>
#endif

namespace Measurements
{
/**
 * @brief Calculates the Pearson correlation coefficient
 *
 * @param u the vector which is checked for correlation, size must be equal to
 * vector v
 * @param v the vector which is checked for correlation, size must be equal to
 * vector u
 * @return [-1]: Inversed correlation
 *          [0]: No correlation at all
 *          [1]: A perfect correlation
 * The return value (RetVal) is within: -1 <= RetVal <= 1
 * as a fraction. The higher the value, the more correlated the data is or the
 * lower the value, the higher the inversed correlation is
 */
double CCorrelation::GetCorrelationCoefficient(const std::vector<double> &u,
                                               const std::vector<double> &v)
{
#if BOOST_VERSION / 100 >= 1072
  return boost::math::statistics::correlation_coefficient(u, v);
#else
  return boost::math::tools::correlation_coefficient(u, v);
#endif
}

} // namespace Measurements