#include "core/logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "core/paths.h"

static std::string timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  // localtime is fine for a single-threaded game logger
  std::tm* tm_info = std::localtime(&t);  // NOLINT(concurrency-mt-unsafe)
  ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

Logger::Logger() {
  logStream_.open(paths::kGameLogPath, std::ios::trunc);
  errStream_.open(paths::kErrorLogPath, std::ios::app);
}

Logger::~Logger() {
  if (logStream_.is_open()) logStream_.close();
  if (errStream_.is_open()) errStream_.close();
}

Logger& Logger::get() {
  static Logger instance;
  return instance;
}

void Logger::log(const std::string& msg) {
  if (logStream_.is_open()) {
    logStream_ << "[" << timestamp() << "] " << msg << "\n";
    logStream_.flush();
  }
}

void Logger::error(const std::string& msg) {
  if (errStream_.is_open()) {
    errStream_ << "[" << timestamp() << "] " << msg << "\n";
    errStream_.flush();
  }
}
