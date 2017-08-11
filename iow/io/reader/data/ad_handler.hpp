#pragma once

#include <iow/io/reader/data/tags.hpp>
#include <memory>

namespace iow{ namespace io{ namespace reader{ namespace data{
  
struct ad_handler
{
  template<typename T>
  void operator()(T& t)
  {
    auto& buf = t.get_aspect().template get<_read_buffer_>();
    while (auto d = buf.detach() )
    {
      if (d->empty() )
        continue;
      t.get_aspect().template get<_input_>()(t, std::move(d) );
    }
  }
};

}}}}
