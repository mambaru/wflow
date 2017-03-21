#pragma once

#include <iow/io/socket/dgram/options.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{
  
struct options:
  ::iow::io::socket::dgram::options
{
  int threads = 0;
  std::string addr;
  std::string port;

};

}}}}

