#pragma once

#include <iow/asio.hpp>
#include <utility>

namespace iow{ namespace io{ namespace socket{ namespace stream{ namespace asio{

struct ad_async_read_some
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H&& handler)
  {
    using namespace std::placeholders;
    t.descriptor().async_read_some(
      ::iow::asio::buffer( p.first, p.second ),
      std::bind(handler, _1, _2, nullptr)
      //std::forward<H>(handler)
    );
  }
};

}}}}}
