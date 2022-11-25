#include "export_graphs.h"

#include <iostream>
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
  for (const auto &e : correlations)
  {
    if (e.correlation > 0.75 || e.correlation < -0.75)
      CreateGraph(e.sensor1, e.sensor2);
  }

  return true;
}

void CGraphs::CreateGraph(const Measurements::Sensors &sensor1,
                          const Measurements::Sensors &sensor2)
{
  FILE *pipe_gp = popen("gnuplot", "w");
  fputs("set terminal png \n", pipe_gp);
  std::string outputStr = "set output 'graphs/" + sensor1.userId + " + " +
                          sensor2.userId + ".png' \n";
  fputs(outputStr.c_str(), pipe_gp);
  //   fputs("set xlabel 'f' \n", pipe_gp);
  fputs("set datafile separator ','\n", pipe_gp);
  fputs("set key autotitle columnhead \n", pipe_gp);
  fputs("set terminal png size 1000, 1000 enhanced font 'Sans,10'\n", pipe_gp);
  fputs("set ytics nomirror\n", pipe_gp);
  fputs("set y2tics \n", pipe_gp);
  std::string plot1 = sensor1.userId;
  std::string plot2 = sensor2.userId;

  std::string plotStr =
      "plot 'Pipeline config.csv' using 'time':'" + plot1 +
      "' with lines axis x1y1, 'Measurement fields.csv' using 'time':'" +
      plot2 +
      "' "
      "with lines axis x1y2 \n";
  fputs(plotStr.c_str(), pipe_gp);
  pclose(pipe_gp);
}

} // namespace Exports