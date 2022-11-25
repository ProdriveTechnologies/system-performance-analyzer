#include "export_graphs.h"

#include <cmath>
#include <iostream>
#include <src/globals.h>
#include <stdio.h>
#include <stdlib.h>

namespace Exports
{
bool CGraphs::FullExport(
    const std::vector<MeasurementItem> &config, const FullMeasurement data,
    const AllSensors &allSensors,
    [[maybe_unused]] const std::vector<Measurements::CCorrelation::SResult>
        &correlations)
{
  allSensors_ = allSensors;
  for (const auto &e : correlations)
  {
    if (std::abs(e.correlation) >= Globals::strongCorrelation)
      CreateGraph(e.sensor1, e.sensor2);
  }

  return true;
}

void CGraphs::CreateGraph(const Measurements::Sensors &sensor1,
                          const Measurements::Sensors &sensor2)
{
  FILE *pipe_gp = popen("gnuplot", "w");
  fputs("set terminal svg \n", pipe_gp);
  std::string outputStr = "set output 'graphs/" + sensor1.userId + " + " +
                          sensor2.userId + ".svg' \n";
  fputs(outputStr.c_str(), pipe_gp);
  //   fputs("set xlabel 'f' \n", pipe_gp);
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

  std::string plotStr = "plot '" + sensor1file + "' using 'time':'" + plot1 +
                        "' with lines axis x1y1, '" + sensor2file +
                        "' using 'time':'" + plot2 +
                        "' "
                        "with lines axis x1y2 \n";
  fputs(plotStr.c_str(), pipe_gp);
  pclose(pipe_gp);
}

/**
 * @brief TODO: remove and replace by configable filename
 *
 * @param c
 * @return std::string
 */
std::string CGraphs::GetFileName(const Measurements::Classification c)
{
  std::string sensorfile;
  if (c == Measurements::Classification::PIPELINE)
    sensorfile = "PipelineMeasurements.csv";
  else if (c == Measurements::Classification::PROCESSES)
    sensorfile = "ProcessMeasurements.csv";
  else
    sensorfile = "SystemResources.csv";
  return sensorfile;
}

} // namespace Exports