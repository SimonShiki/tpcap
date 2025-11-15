#pragma once

#include <iostream>
#include <string>

namespace UnixColor {
constexpr const char *RESET = "\033[0m";
constexpr const char *RED = "\033[31m";
constexpr const char *GREEN = "\033[32m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *BLUE = "\033[34m";
constexpr const char *MAGENTA = "\033[35m";
constexpr const char *CYAN = "\033[36m";
constexpr const char *WHITE = "\033[37m";
} // namespace UnixColor

bool isOutputTTY(std::ostream &out);

void logInfo(const std::string &msg, std::ostream &out = std::cout);
void logWarn(const std::string &msg, std::ostream &out = std::cout);
void logError(const std::string &msg, std::ostream &out = std::cerr);
