#pragma once

#include <iow/ip/tcp/acceptor/options.hpp>
#include <iow/ip/tcp/connection/options.hpp>
#include <thread>

namespace iow{ namespace ip{ namespace tcp{ namespace server{

template<typename AcceptorOptions = ::iow::ip::tcp::acceptor::options<> >  
struct options:  AcceptorOptions 
{
  // TODO: в базовый сервер
  int threads = 0;
  
  typedef std::function<void(std::thread::id)> thread_handler;
  thread_handler thread_startup;
  thread_handler thread_shutdown;

  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler;
  statistics_handler thread_statistics;


};

}}}}

