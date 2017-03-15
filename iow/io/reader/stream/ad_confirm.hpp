#pragma once

#include <iow/io/reader/stream/tags.hpp>
#include <memory>

namespace iow{ namespace io{ namespace reader{ namespace stream{

struct ad_confirm
{
  template<typename T, typename P>
  void operator()(T& t, P p)
  {
    bool result = t.get_aspect().template get<_read_buffer_>().confirm( std::move(p) );
    if ( !result )
    {
      // read error
    }
  }
};

}}}}
