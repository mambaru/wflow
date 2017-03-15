#pragma once

#include <iow/io/socket/stream/socket.hpp>
#include <iow/ip/tcp/connection/aspect.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace connection{

template<typename A = fas::aspect<> >
using connection_base =
  ::iow::io::socket::stream::socket_base< typename fas::merge_aspect<A, aspect >::type >;

template<typename A = fas::aspect<> >
using connection =
  ::iow::io::socket::stream::socket< typename fas::merge_aspect<A, aspect >::type >;


}}}}
