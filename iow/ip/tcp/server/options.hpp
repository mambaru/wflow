#pragma once

#include <iow/ip/tcp/acceptor/options.hpp>
#include <iow/ip/tcp/connection/options.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace server{

template<typename AcceptorOptions = ::iow::ip::tcp::acceptor::options<> >  
struct options:  AcceptorOptions 
{
  // TODO: в базовый сервер
  int threads = 0;
  typedef std::function<void(std::thread::id)> thread_handler;
  thread_handler thread_startup;
  thread_handler thread_shutdown;

};

}}}}

