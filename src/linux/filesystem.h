#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "src/helpers/helper_functions.h"

namespace Linux
{
namespace FileSystem
{
struct Path
{
  std::vector<std::string> pathItems;
  std::string GetPath() const
  {
    std::string path;
    for (const auto &e : pathItems)
      path += "/" + e;
    return path;
  }
  template <typename... Args> void AddItems(const Args &... args)
  {
    (pathItems.push_back(Helpers::GetString(args)), ...);
  }
  void Reset() { pathItems.clear(); }
  bool PathExists() const
  {
    std::ifstream fileObj{GetPath()};
    return fileObj.good();
  }
  //   void insertPath(const std::string& path) {
  //       reset();
  //   }
};

std::vector<std::string> GetFiles(const std::string &path);

struct Stat
{
  int pid;
  std::string exeName;
  char state;
  int ppid;
  int pgrp;
  int session;
  int ttyNr;
  int tpgid;
  long unsigned flags;
  long unsigned minFaults;
  long unsigned cminFaults;
  long unsigned majorFaults;
  long unsigned cmajorFaults;
  long unsigned utime;
  long unsigned stime;
  long int cutime;
  long int cstime;
  long int priority;
  long int nice;
  long int numThreads;
  long int itrealvalue;
  long long unsigned starttime;
  long unsigned vsize;
  long int rss;
  long unsigned rsslim;
  long unsigned startcode;
  long unsigned endcode;
  long unsigned startstack;
  long unsigned kstkesp;
  long unsigned kstkeip;
  long unsigned signal;
  long unsigned blocked;
  long unsigned sigignore;
  long unsigned sigcatch;
  long unsigned wchan;
  long unsigned nswap;
  long unsigned cnswap;
  int exitSignal;
  int processor;
  long unsigned rtPriority;
  long unsigned policy;
  long long unsigned delayAcctBlkioTicks;
  long unsigned guestTime;
  long int cguestTime;
  long unsigned startData;
  long unsigned endData;
  long unsigned startBrk;
  long unsigned argStart;
  long unsigned argEnd;
  long unsigned envStart;
  long unsigned envEnd;
  int exitCode;

  Stat(const std::vector<std::string> &stats_)
      : pid{std::stoi(stats_[0])}, exeName{stats_[1]}, state{stats_[2][0]},
        ppid{std::stoi(stats_[3])}, pgrp{std::stoi(stats_[4])},
        session{std::stoi(stats_[5])}, ttyNr{std::stoi(stats_[6])},
        tpgid{std::stoi(stats_[7])}, flags{std::stoul(stats_[8])},
        minFaults{std::stoul(stats_[9])}, cminFaults{std::stoul(stats_[10])},
        majorFaults{std::stoul(stats_[11])}, cmajorFaults{std::stoul(
                                                 stats_[12])},
        utime{std::stoul(stats_[13])}, stime{std::stoul(stats_[14])},
        cutime{std::stol(stats_[15])}, cstime{std::stol(stats_[16])},
        priority{std::stol(stats_[17])}, nice{std::stol(stats_[18])},
        numThreads{std::stol(stats_[19])}, itrealvalue{std::stol(stats_[20])},
        starttime{std::stoull(stats_[21])}, vsize{std::stoul(stats_[22])},
        rss{std::stoi(stats_[23])}, rsslim{std::stoul(stats_[24])},
        startcode{std::stoul(stats_[25])}, endcode{std::stoul(stats_[26])},
        startstack{std::stoul(stats_[27])}, kstkesp{std::stoul(stats_[28])},
        kstkeip{std::stoul(stats_[29])}, signal{std::stoul(stats_[30])},
        blocked{std::stoul(stats_[31])}, sigignore{std::stoul(stats_[32])},
        sigcatch{std::stoul(stats_[33])}, wchan{std::stoul(stats_[34])},
        nswap{std::stoul(stats_[35])}, cnswap{std::stoul(stats_[36])},
        exitSignal{std::stoi(stats_[37])}, processor{std::stoi(stats_[38])},
        rtPriority{std::stoul(stats_[39])}, policy{std::stoul(stats_[40])},
        delayAcctBlkioTicks{std::stoull(stats_[41])},
        guestTime{std::stoul(stats_[42])}, cguestTime{std::stoi(stats_[43])},
        startData{std::stoul(stats_[44])}, endData{std::stoul(stats_[45])},
        startBrk{std::stoul(stats_[46])}, argStart{std::stoul(stats_[47])},
        argEnd{std::stoul(stats_[48])}, envStart{std::stoul(stats_[49])},
        envEnd{std::stoul(stats_[50])}, exitCode{std::stoi(stats_[51])}
  {
  }
};

Stat GetStats(const std::string &statLocation);

} // namespace FileSystem
} // namespace Linux