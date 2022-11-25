#pragma once

#include <fstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "src/helpers/helper_functions.h"
#include <iostream>
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
  Stat() = default;
  std::unordered_map<std::string,
                     std::variant<int *, long unsigned *, long int *,
                                  long long unsigned *, char *>>
      nameToValue = {{"pid", &pid},
                     {"state", &state},
                     {"ppid", &ppid},
                     {"pgrp", &pgrp},
                     {"session", &session},
                     {"tty_nr", &ttyNr},
                     {"tpgid", &tpgid},
                     {"flags", &flags},
                     {"minflt", &minFaults},
                     {"cminflt", &cminFaults},
                     {"majflt", &majorFaults},
                     {"cmajflt", &cmajorFaults},
                     {"utime", &utime},
                     {"stime", &stime},
                     {"cutime", &cutime},
                     {"cstime", &cstime},
                     {"priority", &priority},
                     {"nice", &nice},
                     {"num_threads", &numThreads},
                     {"itrealvalue", &itrealvalue},
                     {"starttime", &starttime},
                     {"vsize", &vsize},
                     {"rss", &rss},
                     {"rsslim", &rsslim},
                     {"startcode", &startcode},
                     {"endcode", &endcode},
                     {"startstack", &startstack},
                     {"kstkesp", &kstkesp},
                     {"kstkeip", &kstkeip},
                     {"signal", &signal},
                     {"blocked", &blocked},
                     {"sigignore", &sigignore},
                     {"sigcatch", &sigcatch},
                     {"wchan", &wchan},
                     {"nswap", &nswap},
                     {"cnswap", &cnswap},
                     {"exit_signal", &exitSignal},
                     {"processor", &processor},
                     {"rt_priority", &rtPriority},
                     {"policy", &policy},
                     {"delayacct_blkio_ticks", &delayAcctBlkioTicks},
                     {"guest_time", &guestTime},
                     {"cguest_time", &cguestTime},
                     {"start_data", &startData},
                     {"end_data", &endData},
                     {"start_brk", &startBrk},
                     {"arg_start", &argStart},
                     {"arg_end", &argEnd},
                     {"env_start", &envStart},
                     {"env_end", &envEnd},
                     {"exit_code", &exitCode}};
};

Stat GetStats(const std::string &statLocation);

/**
 * @brief Begin of the /proc/stat parsing
 *
 */
struct ProcRow
{
  std::vector<std::string> rowElements;
};
struct ProcStatData
{
  struct Cpu
  {
    long jiffiesUser;
    long jiffiesNice;
    long jiffiesSystem;
    long jiffiesIdle;
    long jiffiesIoWait;
    long jiffiesIrq;
    long jiffiesSoftIrq;
    Cpu operator-(const Cpu &r) const
    {
      Cpu newResult;
      newResult.jiffiesUser = jiffiesUser - r.jiffiesUser;
      newResult.jiffiesNice = jiffiesNice - r.jiffiesNice;
      newResult.jiffiesSystem = jiffiesSystem - r.jiffiesSystem;
      newResult.jiffiesIdle = jiffiesIdle - r.jiffiesIdle;
      newResult.jiffiesIoWait = jiffiesIoWait - r.jiffiesIoWait;
      newResult.jiffiesIrq = jiffiesIrq - r.jiffiesIrq;
      newResult.jiffiesSoftIrq = jiffiesSoftIrq - r.jiffiesSoftIrq;
      return newResult;
    }
    Cpu(const ProcRow &cpuRow)
    {
      if (!Add(cpuRow))
        throw std::runtime_error("/proc/stat CPU data incorrect!");
    }
    Cpu() = default;
    bool Add(const std::vector<std::string> &cpuRow)
    {
      if (cpuRow.size() < 9 || !StartsWithCpu(cpuRow.at(0)))
        return false;
      jiffiesUser = std::stol(cpuRow.at(1));
      jiffiesNice = std::stol(cpuRow.at(2));
      jiffiesSystem = std::stol(cpuRow.at(3));
      jiffiesIdle = std::stol(cpuRow.at(4));
      jiffiesIoWait = std::stol(cpuRow.at(5));
      jiffiesIrq = std::stol(cpuRow.at(6));
      jiffiesSoftIrq = std::stol(cpuRow.at(7));
      return true;
    }
    static bool StartsWithCpu(const std::string &cpuString)
    {
      return cpuString.size() >= 3 && cpuString.substr(0, 3) == "cpu";
    }
    bool Add(const ProcRow &cpuRow) { return Add(cpuRow.rowElements); };
  };
  ProcStatData operator-(const ProcStatData &r)
  {
    ProcStatData newResult;
    newResult.totalCpu = this->totalCpu - r.totalCpu;

    for (const auto &cpuLeft : cpus)
    {
      const auto &cpuRight = r.cpus.find(cpuLeft.first);
      if (cpuRight == r.cpus.end())
        throw std::runtime_error("Field mismatch with CPUs!");
      auto newCpuValues = cpuLeft.second - cpuRight->second;
      newResult.cpus.insert(std::make_pair(cpuLeft.first, newCpuValues));
    }
    return newResult;
  }
  Cpu totalCpu;
  std::map<std::string, Cpu> cpus;
};

ProcStatData GetProcStat(const std::string &procStatLocation);
long long GetProcStatGroup(const ProcStatData::Cpu &cpuField,
                           const std::string &groupName);

/**
 * @brief Begin of the /proc/meminfo parsing
 *
 */
struct MemInfoData
{
  MemInfoData()
      : memtotal{0}, memfree{0}, memavailable{0}, buffers{0}, cached{0},
        swapcached{0}, active{0}, inactive{0}, swaptotal{0}, swapfree{0},
        dirty{0}, fieldMap{{"MemTotal", &memtotal},
                           {"MemFree", &memfree},
                           {"MemAvailable", &memavailable}}
  {
  }
  MemInfoData(const MemInfoData &c)
      : memtotal{c.memtotal}, memfree{c.memfree}, memavailable{c.memavailable},
        buffers{c.buffers}, cached{c.cached}, swapcached{c.swapcached},
        active{c.active}, inactive{c.inactive}, swaptotal{c.swaptotal},
        swapfree{c.swapfree}, dirty{c.dirty}, fieldMap{{"MemTotal", &memtotal},
                                                       {"MemFree", &memfree},
                                                       {"MemAvailable",
                                                        &memavailable}}
  {
  }
  MemInfoData &operator=(const MemInfoData &r)
  {
    memtotal = r.memtotal;
    memfree = r.memfree;
    memavailable = r.memavailable;
    buffers = r.buffers;
    cached = r.cached;
    swapcached = r.swapcached;
    active = r.active;
    inactive = r.inactive;
    swaptotal = r.swaptotal;
    swapfree = r.swapfree;
    dirty = r.dirty;
    return *this;
  }

  unsigned long memtotal;
  unsigned long memfree;
  unsigned long memavailable;
  unsigned long buffers;
  unsigned long cached;
  unsigned long swapcached;
  unsigned long active;
  unsigned long inactive;
  // unsigned long activeAnon = 0;
  // unsigned long inactiveAnon = 0;
  // unsigned long activeFile = 0;
  // unsigned long inactiveFile = 0;
  // unsigned long unevictable = 0;
  // unsigned long mlocked = 0;
  // unsigned long hightotal = 0;
  // unsigned long highfree = 0;
  // unsigned long lowtotal = 0;
  // unsigned long lowfree = 0;
  // unsigned long mmapCopy = 0;
  unsigned long swaptotal;
  unsigned long swapfree;
  unsigned long dirty;
  // unsigned long writeback = 0;
  // unsigned long anonpages = 0;
  // unsigned long mapped = 0;
  // unsigned long shmem = 0;
  // unsigned long kreclaimable = 0;
  // unsigned long slab = 0;
  // unsigned long sreclaimable = 0;
  // unsigned long sunreclaim = 0;
  // unsigned long kernelstack = 0;
  const std::unordered_map<std::string, unsigned long *> fieldMap;
  void ParseRow(const ProcRow &row)
  {
    auto field = fieldMap.find(row.rowElements.at(0));
    if (field != fieldMap.end())
      *(field->second) = std::stoul(row.rowElements.at(1));
  }

  unsigned long GetField(const std::string &value, const bool optional = false)
  {
    auto field = fieldMap.find(value);
    if (field == fieldMap.end())
    {
      if (optional)
        return 1;
      throw std::runtime_error(
          "Field not found in /proc/meminfo! Field name: " + value);
    }
    return *(field->second);
  }
};

MemInfoData GetMemInfo(const std::string &memInfoLocation);

} // namespace FileSystem
} // namespace Linux