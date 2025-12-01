#pragma once

#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <iostream>
#include <memory>

namespace siren {
namespace Logger {

// 初始化器类
class LoggerInit {
public:
  LoggerInit();

  std::shared_ptr<LoggerInit> getLogger();
  std::shared_ptr<LoggerInit> addLoggerPath();
};

// 全局静态初始化器
namespace {
LoggerInit initializer;
}
#define SIREN_LOG_TRACE                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::trace) \
      << boost::log::add_value("Line", __LINE__)                                         \
      << boost::log::add_value("File", __FILE__)                                         \
      << boost::log::add_value("Function", __FUNCTION__)

#define SIREN_LOG_DEBUG                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::debug) \
      << boost::log::add_value("Line", __LINE__)                                         \
      << boost::log::add_value("File", __FILE__)                                         \
      << boost::log::add_value("Function", __FUNCTION__)

#define SIREN_LOG_INFO                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::INFO) \
      << boost::log::add_value("Line", __LINE__)                                        \
      << boost::log::add_value("File", __FILE__)                                        \
      << boost::log::add_value("Function", __FUNCTION__)

#define SIREN_LOG_WARNING                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::warning) \
      << boost::log::add_value("Line", __LINE__)                                           \
      << boost::log::add_value("File", __FILE__)                                           \
      << boost::log::add_value("Function", __FUNCTION__)

#define SIREN_LOG_ERROR                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::error) \
      << boost::log::add_value("Line", __LINE__)                                         \
      << boost::log::add_value("File", __FILE__)                                         \
      << boost::log::add_value("Function", __FUNCTION__)

#define SIREN_LOG_FATAL                                                                  \
  BOOST_LOG_STREAM_SEV((boost::log::trivial::logger::get()), boost::log::trivial::fatal) \
      << boost::log::add_value("Line", __LINE__)                                         \
      << boost::log::add_value("File", __FILE__)                                         \
      << boost::log::add_value("Function", __FUNCTION__)

}  // namespace Logger
}  // namespace siren