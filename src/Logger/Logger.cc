#include "Logger.h"

#include <memory>

siren::Logger::LoggerInit::LoggerInit() {
  // 设置控制台输出
  // clang-format off
  boost::log::add_console_log(
    std::cout, 
    boost::log::keywords::format = (
      boost::log::expressions::stream 
        << "["
        << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S") 
        << "]"
        << " [" 
        << boost::log::trivial::severity 
        << "] " 
        << boost::log::expressions::attr<std::string>("File") 
        << ":" 
        << boost::log::expressions::attr<int>("Line") 
        << " [" 
        << boost::log::expressions::attr<std::string>("Function")
        << "] "
        << boost::log::expressions::smessage));
  // clang-format on
  boost::log::add_common_attributes();
  boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::trace);
}
std::shared_ptr<siren::Logger::LoggerInit> siren::Logger::LoggerInit::getLogger() {
  return std::shared_ptr<siren::Logger::LoggerInit>(this);
}