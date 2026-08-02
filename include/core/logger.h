#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

class Logger {
 public:
  /**
   * @brief Get the singleton instance of the Logger.
   *
   * @return Logger&
   */
  static Logger& get();

  /**
   * @brief Log a message to the general log file.
   *
   * @param msg The message to log.
   */
  void log(const std::string& msg);

  /**
   * @brief Log an error message to the error log file.
   *
   * @param msg The error message to log.
   */
  void error(const std::string& msg);

 private:
  Logger();
  ~Logger();

  std::ofstream logStream_;
  std::ofstream errStream_;
};

/**
 * @brief Convenience macros for logging.
 *
 * LOG(msg) — logs to game.log
 * LOG_ERR(msg) — logs to error.log
 */
#define LOG(msg) Logger::get().log(msg)
#define LOG_ERR(msg) Logger::get().error(msg)

#endif
