//
// Created by 4c3y on 18.08.22.
//

#ifndef LOG__LOG_H_
#define LOG__LOG_H_

#include <algorithm>
#include <utility>
#include <set>
#include <sstream>

//! Check if libraries are available at compile time
#if __has_include(<glog/logging.h>)
#define GLOG_SUPPORTED
#endif

#if __has_include(<ros/console.h>)
#define ROSLOG_SUPPORTED
#endif

//! Initialization logic
namespace lpp {
namespace internal {

class Init {
 public:
  bool is_lpp_initialized = false;
  bool is_glog_initialized = false;
};
inline Init lppInit;
}
}

//! assert if mode is supported by the libraries available
#if defined MODE_GLOG && !defined GLOG_SUPPORTED
#error Logging Mode is set to glog but glog was not found
#endif

#if defined MODE_ROSLOG && !defined ROSLOG_SUPPORTED
#error Logging Mode is set to roslog but roslog was not found
#endif


//! Includes
#if defined GLOG_SUPPORTED && defined MODE_GLOG
#include <glog/logging.h>
#endif // GLOG_SUPPORTED

#if defined ROSLOG_SUPPORTED && defined MODE_ROSLOG
#include <ros/console.h>
#endif


//! un-define macros to avoid conflicts
#ifdef GLOG_SUPPORTED
#undef LOG
#endif

#if defined ROSLOG_SUPPORTED && !defined MODE_ROSLOG
#undef ROS_INFO
#undef ROS_INFO_STREAM
#undef ROS_WARN
#undef ROS_WARN_STREAM
#undef ROS_ERROR
#undef ROS_ERROR_STREAM
#undef ROS_FATAL
#undef ROS_FATAL_STREAM
#endif

using namespace lpp::internal;
//! Log init
#ifdef MODE_GLOG

//! If LOG_INIT is called more than once, do nothing.
#define LOG_INIT(argv0) if (!lppInit.is_glog_initialized) { \
google::InitGoogleLogging(argv0); lppInit.is_glog_initialized = true;} \
lppInit.is_lpp_initialized = true; FLAGS_logtostderr = true
#else
#define LOG_INIT(argv0) lppInit.is_lpp_initialized = true
#endif


//! Hack to enable macro overloading. Used to overload glog's LOG() macro.
#define CAT(A, B) A ## B
#define SELECT(NAME, NUM) CAT( NAME ## _, NUM )

#define GET_COUNT(_1, _2, _3, _4, _5, _6, COUNT, ...) COUNT
#define VA_SIZE(...) GET_COUNT( __VA_ARGS__, 6, 5, 4, 3, 2, 1 )
#define VA_SELECT(NAME, ...) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)


//! Overloads
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define LOG(...) VA_SELECT( LOG, __VA_ARGS__ )
#pragma clang diagnostic pop

/**
 * LOG_1 = Google logging syntax
 * LOG_2 = LPP logging syntax
 */

//! MODE_GLOG
#if defined MODE_GLOG || defined MODE_DEFAULT
#define LOG_1(severity) COMPACT_GOOGLE_LOG_ ## severity.stream()
#endif

#ifdef MODE_GLOG
#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"

#define LOG_2(severity, x) \
if      (strcmp(#severity, "I") == 0) {LOG_1(INFO) << x;}        \
else if (strcmp(#severity, "W") == 0) {LOG_1(WARNING) << x;}     \
else if (strcmp(#severity, "E") == 0) {LOG_1(ERROR) << x;}       \
else if (strcmp(#severity, "F") == 0) {LOG_1(FATAL) << x;}       \

#define ROS_INFO(x) LOG(INFO) << x
#define ROS_INFO_STREAM(x) LOG(INFO) << x
#define ROS_WARN(x) LOG(WARNING) << x
#define ROS_WARN_STREAM(x) LOG(WARNING) << x
#define ROS_ERROR(x) LOG(ERROR) << x
#define ROS_ERROR_STREAM(x) LOG(ERROR) << x
#define ROS_FATAL(x) LOG(FATAL) << x
#define ROS_FATAL_STREAM(x) LOG(FATAL) << x;
#pragma clang diagnostic pop
#endif


//! MODE_ROSLOG
#ifdef MODE_ROSLOG

#define LOG_1(severity) InternalLog(#severity)
#define LOG_2(severity, x) InternalLog(#severity) << x

#endif



//! MODE_LPP
#ifdef MODE_LPP

#define ROS_INFO(x) LOG_2(I, x)
#define ROS_INFO_STREAM(x) LOG_2(I, x)
#define ROS_WARN(x) LOG_2(W, x)
#define ROS_WARN_STREAM(x) LOG_2(W, x)
#define ROS_ERROR(x) LOG_2(E, x)
#define ROS_ERROR_STREAM(x) LOG_2(E, x)
#define ROS_FATAL(x) LOG_2(F, x)
#define ROS_FATAL_STREAM(x) LOG_2(F, x)

#define LOG_1(severity) InternalLog(#severity)
#define LOG_2(severity, x) InternalLog(#severity) << x // NOLINT(bugprone-macro-parentheses)
#endif

enum SeverityType {
  INFO,
  WARN,
  ERROR,
  FATAL
};

//! Internal log class
class InternalLog {
 public:
  explicit InternalLog(SeverityType severity_type) : severity_(severity_type) {}
  explicit InternalLog(const std::string &severity) {
    severity_ = getSeverityFromString(severity);
  }

  SeverityType severity_;
  std::stringstream ss;
#ifdef MODE_ROSLOG
  ~InternalLog() {
    switch (severity_) {
      case SeverityType::INFO:ROS_INFO_STREAM(ss.str());
        break;
      case SeverityType::WARN:ROS_WARN_STREAM(ss.str());
        break;
      case SeverityType::ERROR:ROS_ERROR_STREAM(ss.str());
        break;
      case SeverityType::FATAL:ROS_FATAL_STREAM(ss.str());
        break;
    }
  }
#endif

#ifdef MODE_LPP
  ~InternalLog() {
    switch (severity_) {
      case SeverityType::INFO:std::cout << "INFO  " << ss.str() << std::endl;
        break;
      case SeverityType::WARN:std::cout << "WARN  " << ss.str() << std::endl;
        break;
      case SeverityType::ERROR:std::cout << "ERROR " << ss.str() << std::endl;
        break;
      case SeverityType::FATAL:std::cout << "FATAL " << ss.str() << std::endl;
        break;
    }
  }
#endif

 private:
  static SeverityType getSeverityFromString(const std::string &str) {
    if (INFO.find(str) != INFO.end()) {
      return SeverityType::INFO;
    } else if (WARNING.find(str) != WARNING.end()) {
      return SeverityType::WARN;
    } else if (ERROR.find(str) != ERROR.end()) {
      return SeverityType::ERROR;
    } else if (FATAL.find(str) != FATAL.end()) {
      return SeverityType::FATAL;
    }
    abort();
  }

  inline static const std::set<std::string> INFO{"I", "INFO"};
  inline static const std::set<std::string> WARNING{"W", "WARNING"};
  inline static const std::set<std::string> ERROR{"E", "ERROR"};
  inline static const std::set<std::string> FATAL{"F", "FATAL"};
};

template<typename T>
InternalLog &&operator<<(InternalLog &&wrap, T const &whatever) {
  wrap.ss << whatever;
  return std::move(wrap);
}

#endif //LOG__LOG_H_
