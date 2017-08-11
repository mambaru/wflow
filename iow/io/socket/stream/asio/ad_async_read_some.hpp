#pragma once

#include <iow/asio.hpp>
#include <iow/system.hpp>
#include <utility>

namespace iow{ namespace io{ namespace socket{ namespace stream{ namespace asio{

struct ad_async_read_some1
{
  typedef std::function<void(::iow::system::error_code, std::size_t)> user_handler;
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H&& handler) const
  {
    using namespace std::placeholders;
    t.descriptor().async_read_some(
      ::iow::asio::buffer( p.first, p.second ),
      std::bind(handler, _1, _2, user_handler())
      //std::forward<H>(handler)
    );
  }
};

}}}}}
