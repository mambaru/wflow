#pragma once

#include <iow/io/socket/dgram/options.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{
  
struct options:
  ::iow::io::socket::dgram::options
{
  typedef std::function<void(std::thread::id)> thread_handler;
  thread_handler thread_startup;
  thread_handler thread_shutdown;

  int threads = 0;
  std::string addr;
  std::string port;

  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler;
  statistics_handler thread_statistics;

};

}}}}

