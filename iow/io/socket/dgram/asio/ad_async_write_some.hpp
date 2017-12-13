#pragma once

#include <iow/asio.hpp>
#include <utility>
#include <iow/io/types.hpp>

namespace iow{ namespace io{ namespace socket{ namespace dgram{ namespace asio{

struct ad_async_write_some
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H&& handler) const
  {
    using namespace std::placeholders;
    auto ep = t.get_aspect().template get<_current_endpoint_>();
    t.descriptor().async_send_to( 
      ::boost::asio::buffer( p.first, p.second ),
      *ep, std::forward<H>(handler));
  }
};

}}}}}
