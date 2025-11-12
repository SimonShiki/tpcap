#include <fstream>
#include <iostream>

#include "log.h"

static bool isOfstream(std::ios &ios) {
  return dynamic_cast<std::fstream *>(&ios) != nullptr ||
         dynamic_cast<std::ofstream *>(&ios) != nullptr;
}

// Disable unix color codes when outputting to files

void logInfo(const std::string &msg, std::ostream &out) {
  if (isOfstream(out)) {
    out << "[INFO] " << msg << std::endl;
  } else {
    out << UnixColor::GREEN << "[INFO] " << UnixColor::RESET << msg
        << std::endl;
  }
}

void logWarn(const std::string &msg, std::ostream &out) {
  if (isOfstream(out)) {
    out << "[WARN] " << msg << std::endl;
  } else {
    out << UnixColor::YELLOW << "[WARN] " << UnixColor::RESET << msg
        << std::endl;
  }
}

void logError(const std::string &msg, std::ostream &out) {
  if (isOfstream(out)) {
    out << "[ERROR] " << msg << std::endl;
  } else {
    out << UnixColor::RED << "[ERROR] " << UnixColor::RESET << msg << std::endl;
  }
}
