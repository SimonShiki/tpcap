#include <atomic>
#include <csignal>
#include <fstream>

#include "netcap.h"
#include "log.h"

#define TPCAP_VERSION "0.0.1"

std::atomic<bool> g_running{true};

void quit (int signum) {
    logInfo("Program gratefully stopping...");
    g_running = false;
}

int main (int argc, char* argv[]) {
    if (argc < 2) {
	logError("Wrong arg count. Usage: ./tpcap [LOGFILE_PATH] [OPTIONAL_INTERFACE_NAME]");
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
    netcap.dispose();
    logInfo("Closing log file...");
    logFile.close();

    return 0;
}
