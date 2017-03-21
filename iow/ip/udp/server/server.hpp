#pragma once

#include <iow/io/socket/dgram/socket.hpp>
#include <iow/ip/udp/connection/aspect.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{

template<typename A = fas::aspect<> >
using server_base =
  ::iow::io::socket::dgram::socket_base< typename fas::merge_aspect<A, aspect >::type >;

template<typename A = fas::aspect<> >
using server =
  ::iow::io::socket::dgram::socket< typename fas::merge_aspect<A, aspect >::type >;


}}}}
