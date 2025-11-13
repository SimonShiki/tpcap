#include <atomic>
#include <csignal>
#include <fstream>

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
  if (argc < 2) {
    logError("Wrong arg count. Usage: ./tpcap [LOGFILE_PATH] "
             "[OPTIONAL_INTERFACE_NAME]");
    return 1;
  }

  // Handle signals
  signal(SIGTERM, quit);
  signal(SIGINT, quit);

  // Open log file
  std::ofstream logFile;
  logFile.open(argv[1], std::ios::out | std::ios::app);
  if (!logFile.is_open()) {
    logError("Failed to open log file: " + std::string(argv[1]));
    return 1;
  }

  NetCap netcap(&logFile);
  g_netcap = &netcap;

  if (argc >= 3) {
    netcap.setInterfaceName(argv[2]);
  }

  logInfo("TPCap version " TPCAP_VERSION);

  netcap.init();

  logInfo("Listening on interface: " + netcap.getInterfaceName());
  logInfo("Logging to file: " + std::string(argv[1]));
  logInfo("Press Ctrl+C to stop.\n");

  netcap.startCapture(g_running);

  logInfo("Releasing TPCap...");
  g_netcap = nullptr;
  netcap.dispose();
  logInfo("Closing log file...");
  logFile.close();

  return 0;
}
