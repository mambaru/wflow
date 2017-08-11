#pragma once

#include <iow/io/writer/data/tags.hpp>
#include <utility>


namespace iow{ namespace io{ namespace writer{ namespace data{

struct ad_confirm
{
  template<typename T, typename P>
  void operator()(T& t, P p) const
  {
    bool result = t.get_aspect().template get<_write_buffer_>().confirm(p);
    if ( !result )
    {
      // Все закртыть, и выдать в log
    }
  }
};

}}}}
