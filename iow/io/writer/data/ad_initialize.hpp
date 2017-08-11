#pragma once

#include <iow/io/writer/data/tags.hpp>

namespace iow{ namespace io{ namespace writer{ namespace data{
  
struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, const O& opt) const
  {
    t.get_aspect().template get<_write_buffer_>().set_options(opt);
  }
};


}}}}
