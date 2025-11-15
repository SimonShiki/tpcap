#include <iostream>
#include <unistd.h>

#include "log.h"

inline int getFD(std::ostream &os) {
  if (&os == &std::cout) return fileno(stdout);
  if (&os == &std::cerr) return fileno(stderr);
  if (&os == &std::clog) return fileno(stderr);

  // Otherwise it never be a TTY
  return -1;
}

// Check if output stream is a TTY (terminal) or a pipe/file
inline bool isOutputTTY(std::ostream &out) {
    int fd = getFD(out);

    if (fd < 0)
      return false;

    return ::isatty(fd) != 0;
}

// Disable unix color codes when outputting to files or pipes

void logInfo(const std::string &msg, std::ostream &out) {
  if (!isOutputTTY(out)) {
    out << "[INFO] " << msg << std::endl;
  } else {
    out << UnixColor::GREEN << "[INFO] " << UnixColor::RESET << msg
        << std::endl;
  }
}

void logWarn(const std::string &msg, std::ostream &out) {
  if (!isOutputTTY(out)) {
    out << "[WARN] " << msg << std::endl;
  } else {
    out << UnixColor::YELLOW << "[WARN] " << UnixColor::RESET << msg
        << std::endl;
  }
}

void logError(const std::string &msg, std::ostream &out) {
  if (!isOutputTTY(out)) {
    out << "[ERROR] " << msg << std::endl;
  } else {
    out << UnixColor::RED << "[ERROR] " << UnixColor::RESET << msg << std::endl;
  }
}
