#include <atomic>
#include <csignal>
#include <fstream>
#include <unistd.h>

#include "log.h"
#include "netcap.h"

#define TPCAP_VERSION "0.0.1"

std::atomic<bool> g_running{true};
NetCap *g_netcap = nullptr;

void quit(int signum) {
  if (!g_running.exchange(false)) {
    return;
  }
  logInfo("Program gracefully stopping...");
  
  // Break out of pcap_dispatch immediately
  if (g_netcap != nullptr) {
    g_netcap->breakCapture();
  }
}

int main(int argc, char *argv[]) {
  bool usePipe = false;
  std::ostream *logStream = nullptr;
  std::ofstream logFile;
  std::string ifaceName;

  // Check if stdout is a pipe
  if (!isatty(STDOUT_FILENO)) {
    usePipe = true;
    logStream = &std::cout;

    // Check if stdin is also a pipe, then read interface name from stdin
    if (!isatty(STDIN_FILENO)) {
      std::getline(std::cin, ifaceName);
    }
  } else {
    if (argc < 2) {
      logError("Wrong arg count. Usage: ./tpcap [LOGFILE_PATH] "
               "[OPTIONAL_INTERFACE_NAME]");
      logError("Or use pipe: echo [INTERFACE_NAME] | ./tpcap | <consumer>");
      return 1;
    }


    logFile.open(argv[1], std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
      logError("Failed to open log file: " + std::string(argv[1]));
      return 1;
    }
    logStream = &logFile;
    
    if (argc >= 3) {
      ifaceName = argv[2];
    }
  }

  // Handle signals
  signal(SIGTERM, quit);
  signal(SIGINT, quit);

  NetCap netcap(logStream);
  g_netcap = &netcap;

  if (!ifaceName.empty()) {
    netcap.setInterfaceName(ifaceName);
  }

  logInfo("TPCap version " TPCAP_VERSION);

  netcap.init();

  logInfo("Listening on interface: " + netcap.getInterfaceName());
  if (usePipe) {
    logInfo("Logging to stdout");
  } else {
    logInfo("Logging to file: " + std::string(argv[1]));
  }
  logInfo("Press Ctrl+C to stop.\n");

  netcap.startCapture(g_running);

  logInfo("Releasing TPCap...");
  g_netcap = nullptr;
  netcap.dispose();
  
  if (!usePipe) {
    logInfo("Closing log file...");
    logFile.close();
  }

  return 0;
}
