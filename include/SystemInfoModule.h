/*
 * SystemInfo.h
 *
 *  Created on: Sep 6, 2017
 *      Author: zenker
 */

#ifndef INCLUDE_SYSTEMINFOMODULE_H_
#define INCLUDE_SYSTEMINFOMODULE_H_

#undef GENERATE_XML
#include <ChimeraTK/ApplicationCore/ApplicationCore.h>

#include "Logging.h"

#include "sys_stat.h"
#include <unordered_set>

namespace ctk = ChimeraTK;

/**
 * \brief This module is used to read system parameter.
 *
 * Some of them are static and only read once (e.g. processor model).
 * Others are watched continuously (e.g. uptime, work load...).
 * \todo Implement proper data types instead of using int for all of them!
 */
class SystemInfoModule: public ctk::ApplicationModule {
private:
  SysInfo sysInfo;

  /**
   * CPU usage parameters needed to calculate the cpu usage.
   * These vales are read from \c /proc/stat
   */
  struct cpu {
    unsigned long long totalUser;
    unsigned long long totalUserLow;
    unsigned long long totalSys;
    unsigned long long totalIdle;
    cpu(unsigned long long totUser = 0, unsigned long long totUserLow = 0,
        unsigned long long TotSys = 0, unsigned long long TotIdle = 0) {
      totalUser = totUser;
      totalUserLow = totUserLow;
      totalSys = TotSys;
      totalIdle = TotIdle;
    }
  };

  /**
   * CPU usage parameters (see cpu) for the total system and the individual cores.
   * Therefore, the size of this vector is nCores + 1
   */
  std::vector<cpu> lastInfo;

  /**
   * Calculates the percentage of cpu usage.
   * This is done in total and for core found on the system.
   * If cpu usage is set to -1 there was an overflow in the \c /proc/stat file.
   */
  void calculatePCPU();

  /**
   * Read values from the \c /proc/stat for all cpu cores (cpux) and overall values (cpu).
   */
  void readCPUInfo(std::vector<cpu> &vcpu);

public:
  SystemInfoModule(EntityOwner *owner, const std::string &name,
      const std::string &description, bool eliminateHierarchy = false,
      const std::unordered_set<std::string> &tags = { });

  ctk::ScalarPushInput<uint64_t> trigger { this, "trigger", "",
      "Trigger used to update the watchdog" };
  /**
   * \name Static system information (read only once)
   * @{
   */
  std::map<std::string, ctk::ScalarOutput<std::string> > strInfos;
  ctk::ScalarOutput<int> ticksPerSecond { this, "ticksPerSecond", "Hz",
      "Number of clock ticks per second" }; ///< Number of clock ticks per second
  ctk::ScalarOutput<int> nCPU { this, "nCPU", "", "Number of CPUs",
    { "CS", "SYS" } };
  /** @} */
  /**
   * \name Non static system information
   * @{
   */
  //\todo: Implement the following as unsigned long!
  ctk::ScalarOutput<int> maxMem { this, "maxMem", "kB",
      "Maximum available memory", { "CS", "SYS" } };
  ctk::ScalarOutput<int> freeMem { this, "freeMem", "kB", "Free memory",
    { "CS", "SYS", "DAQ" } };
  ctk::ScalarOutput<int> cachedMem { this, "cachedMem", "kB", "Cached memory",
    { "CS", "SYS" } };
  ctk::ScalarOutput<int> usedMem { this, "usedMem", "kB", "Used memory",
    { "CS", "SYS", "DAQ"} };
  ctk::ScalarOutput<int> maxSwap { this, "maxSwap", "kB", "Swap size",
    { "CS", "SYS" } };
  ctk::ScalarOutput<int> freeSwap { this, "freeSwap", "kB", "Free swap",
    { "CS", "SYS", "DAQ" } };
  ctk::ScalarOutput<int> usedSwap { this, "usedSwap", "kB", "Used swap",
    { "CS", "SYS", "DAQ" } };
  //\todo: Implement the following as long!
  ctk::ScalarOutput<int> startTime { this, "startTime", "s", "start time of system with respect to EPOCH",
      { "CS", "SYS" } };
  ctk::ScalarOutput<std::string> startTimeStr { this, "startTimeStr", "", "startTimeStr",
        { "CS", "SYS" } };
  ctk::ScalarOutput<int> uptime_secTotal { this, "uptimeSecTotal", "s", "Total uptime",
      { "CS", "SYS" } };
  ctk::ScalarOutput<int> uptime_day { this, "uptimeDays", "day", "Days up",
    { "CS", "SYS" } };
  ctk::ScalarOutput<int> uptime_hour { this, "uptimeHours", "h", "Hours up",
    { "CS", "SYS" } };
  ctk::ScalarOutput<int> uptime_min { this, "uptimeMin", "min", "Minutes up",
    { "CS", "SYS" } };
  ctk::ScalarOutput<int> uptime_sec { this, "uptimeSec", "s", "Seconds up",
    { "CS", "SYS" } };
  std::unique_ptr<ctk::ArrayOutput<double> > cpu_use;
  ctk::ScalarOutput<double> cpu_useTotal {this, "cpuTotal", "%", "Total CPU usage",
    { "CS", "SYS", "DAQ" } };
  ctk::ArrayOutput<double> loadAvg{ this, "loadAvg", "", 3, "Average load within last min, 5min, 15min",
    { "CS", "SYS", "DAQ" } };
  /** @} */

  /**
   * \name Logging
   * @{
   */
  std::ostream *logging;
#ifdef ENABLE_LOGGING
  /** Message to be send to the logging module */
  ctk::ScalarOutput<std::string> message { this, "message", "", "Message of the module to the logging System",
      { "Logging", "PROCESS", getName() } };

  /** Message to be send to the logging module */
  ctk::ScalarOutput<uint> messageLevel { this, "messageLevel", "", "Logging level of the message",
      { "Logging", "PROCESS", getName() } };

  void sendMessage(const logging::LogLevel &level = logging::LogLevel::INFO);

#endif
  /** @} */

  /**
   * Main loop function.
   * Reads number of cores and system clock ticks and other static parameter only once before the loop.
   */
  void mainLoop() override;

  /**
   * Clean up ostream pointer and terminate the application module.
   */
  void terminate() override;



};

/*
 * Return the current time in a formatted way:
 * WATCHDOG_SERVER: 2018-Oct-25 11:22:29.411611  -> "module name" ->
 * This is used to be prepended to logging messages.
 */
std::string getTime(ctk::ApplicationModule* mod);

extern std::mutex fs_mutex; ///< This mutex is used when reading filesystem information via statfs

/**
 * \brief Module reading file system information (size, available space)
 *
 * Hard disk devices are mounted at a certain location. The statfs that is used
 * internally to read information needs the mount point and not the device itself.
 * E.g. /dev/sdb1 mounted at /media/data -> statfs works on /media/data.
 * Therefore the module might be used multiple times for different mount point to
 * be monitored.
 */
struct FileSystemModule : public ctk::ApplicationModule {
  /**
   * Constructor.
   * \param devName The name of the device (e.g. /dev/sdb1 -> sdb1)
   * \param mntPoint The mount point of the device (e.g. / for the root directory)
   */
  FileSystemModule(const std::string &devName, const std::string &mntPoint, EntityOwner *owner, const std::string &name,
        const std::string &description, bool eliminateHierarchy = false,
        const std::unordered_set<std::string> &tags = { });

  /**
   * Publish the device name although it is also encoded in the path by
   * the watchdog  sever.
   */
  ctk::ScalarOutput<std::string> deviceName { this, "deviceName", "", "Name of the device",
        { "CS", "SYS"} };
  ctk::ScalarOutput<std::string> mountPoint { this, "mountPoint", "", "Mount point of the device",
          { "CS", "SYS"} };
  ctk::ScalarOutput<double> disk_size { this, "size", "GiB", "Mount point of the device",
          { "CS", "SYS", "DAQ"} };
  ctk::ScalarOutput<double> disk_free { this, "free", "GiB", "Free disk space",
          { "CS", "SYS", "DAQ"} };
  ctk::ScalarOutput<double> disk_user { this, "freeUser", "GiB", "Free disk space available for the user",
          { "CS", "SYS", "DAQ"} };
  ctk::ScalarOutput<double> disk_usage { this, "usage", "%", "Disk usage with respect to the space available to the user",
          { "CS", "SYS", "DAQ"} };
//
  ctk::ScalarPushInput<uint64_t> trigger { this, "trigger", "",
      "Trigger used to update the watchdog" };

  const double GiB = 1./ 1024/1024/1024; ///< Conversion to GiB (not to be mixed up with GB!)

  std::string tmp[2]; ///<Temporary store device name and mount point
  /**
   * \name Logging
   * @{
   */
  std::ostream *logging;
#ifdef ENABLE_LOGGING
  /** Message to be send to the logging module */
  ctk::ScalarOutput<std::string> message { this, "message", "", "Message of the module to the logging System",
      { "Logging", "SYS", getName() } };

  /** Message to be send to the logging module */
  ctk::ScalarOutput<uint> messageLevel { this, "messageLevel", "", "Logging level of the message",
      { "Logging", "SYS", getName() } };

  void sendMessage(const logging::LogLevel &level = logging::LogLevel::INFO);

#endif
  /** @} */

  /**
   * Main loop function.
   */
  void mainLoop() override;

  /**
   * Clean up ostream pointer and terminate the application module.
   */
  void terminate() override;

  /**
   * Use statfs to read information about the device.
   * Since this might not be thread safe a mutex is used here.
   */
  bool read();
};

/**
 * \brief Module reading information of a network adapter.
 *
 * Parameters that are observed are:
 * - received data (byte, packates)
 * - transmitted data (byte, packates)
 * - dropped data (transmitted and received)
 * - collisions
 *
 *
 * Hard disk devices are mounted at a certain location. The statfs that is used
 * internally to read information needs the mount point and not the device itself.
 * E.g. /dev/sdb1 mounted at /media/data -> statfs works on /media/data.
 * Therefore the module might be used multiple times for different mount point to
 * be monitored.
 */
struct NetworkModule : public ctk::ApplicationModule {
  NetworkModule(const std::string &device, EntityOwner *owner, const std::string &name,
        const std::string &description, bool eliminateHierarchy = false,
        const std::unordered_set<std::string> &tags = { });

  /**
   * Publish the device name although it is also encoded in the path by
   * the watchdog  sever.
   */
  ctk::ScalarOutput<std::string> deviceName { this, "device", "", "Name of the device",
        { "CS", "SYS"} };

  /*
   * Use a vector to handle all output variables of the module for easy
   * filling.
   */
  std::vector<ctk::ScalarOutput<double> > data;

  ctk::ScalarPushInput<uint64_t> trigger { this, "trigger", "",
      "Trigger used to update the watchdog" };

  const double MiB = 1./ 1024/1024; ///< Conversion to MiB (not to be mixed up with MB!)

  std::string tmp; ///<Temporary store device name

  struct raw{
    std::vector<unsigned long long> data; ///< Used to store raw data read from sys files
    std::vector<std::string> files;///< Names of the sys files used to get networking information
    std::vector<boost::posix_time::ptime> time;///< Safe individual time stamps of reading a certain sys file
    raw(const std::string &device){
      data = std::vector<unsigned long long>(7);
      std::string src_base("/sys/class/net/");
      files.push_back(src_base+device+"/statistics/rx_packets");
      files.push_back(src_base+device+"/statistics/tx_packets");
      files.push_back(src_base+device+"/statistics/rx_bytes");
      files.push_back(src_base+device+"/statistics/tx_bytes");
      files.push_back(src_base+device+"/statistics/rx_dropped");
      files.push_back(src_base+device+"/statistics/tx_dropped");
      files.push_back(src_base+device+"/statistics/collisions");
      time = std::vector<boost::posix_time::ptime>(7);
    }
    raw() = default;
  };

  raw previousData; ///< Last data collected

  /**
   * \name Logging
   * @{
   */
  std::ostream *logging;
#ifdef ENABLE_LOGGING
  /** Message to be send to the logging module */
  ctk::ScalarOutput<std::string> message { this, "message", "", "Message of the module to the logging System",
      { "Logging", "SYS", getName() } };

  /** Message to be send to the logging module */
  ctk::ScalarOutput<uint> messageLevel { this, "messageLevel", "", "Logging level of the message",
      { "Logging", "SYS", getName() } };

  void sendMessage(const logging::LogLevel &level = logging::LogLevel::INFO);

#endif
  /** @} */

  /**
   * Main loop function.
   */
  void mainLoop() override;

  /**
   * Clean up ostream pointer and terminate the application module.
   */
  void terminate() override;

  /**
   * Use statfs to read information about the device.
   * Since this might not be thread safe a mutex is used here.
   */
  bool read();
};

#endif /* INCLUDE_SYSTEMINFOMODULE_H_ */
