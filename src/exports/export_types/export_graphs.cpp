#include "export_graphs.h"

#include "src/benchmarks/linux/struct_sensors.h"

#include <cmath> // std::abs
#include <src/globals.h>
#include <stdio.h> // fputs, FILE

namespace Exports
{
bool CGraphs::FullExport([[maybe_unused]] const std::vector<SMeasurementItem>& config,
                         [[maybe_unused]] const FullMeasurement data,
                         const AllSensors& allSensors,
                         [[maybe_unused]] const std::vector<Measurements::CCorrelation::SResult>& correlations)
{
  allSensors_ = allSensors;
  for (const auto& e : correlations)
  {
    if (std::abs(e.correlation) >= Globals::strongCorrelation)
      CreateGraph(e.sensor1, e.sensor2);
  }

  return true;
}

void CGraphs::CreateGraph(const Measurements::SSensors& sensor1, const Measurements::SSensors& sensor2)
{
  FILE* pipe_gp = popen("gnuplot", "w");
  fputs("set terminal png \n", pipe_gp);
  std::string outputStr = "set output 'graphs/" + sensor1.userId + " + " + sensor2.userId + ".png' \n";
  fputs(outputStr.c_str(), pipe_gp);

  fputs("set datafile separator ','\n", pipe_gp);
  fputs("set key autotitle columnhead \n", pipe_gp);
  fputs("set terminal png size 1000, 1000 enhanced font 'Sans,10'\n", pipe_gp);
  fputs("set ytics nomirror\n", pipe_gp);
  fputs("set y2tics \n", pipe_gp);
  std::string plot1 = sensor1.userId;
  std::string plot2 = sensor2.userId;

  auto classification = allSensors_.GetClassification(sensor1.uniqueId);
  std::string sensor1file = GetFileName(classification);
  classification = allSensors_.GetClassification(sensor2.uniqueId);
  std::string sensor2file = GetFileName(classification);

  std::string plotStr = "plot '" + sensor1file + "' using 'time':'" + plot1 + "' with lines axis x1y1, '" +
                        sensor2file + "' using 'time':'" + plot2 +
                        "' "
                        "with lines axis x1y2 \n";

  fputs(plotStr.c_str(), pipe_gp);
  pclose(pipe_gp);
}

/**
 * @brief Gets the filename from the CSV file (as this export depends on the CSV export)
 * @note The name of the CSV file export and this function must both depend on GetClassificationStr() for proper
 * functioning
 */
std::string CGraphs::GetFileName(const Measurements::EClassification c)
{
  return Measurements::GetClassificationStr(c) + ".csv";
}

} // namespace Exports