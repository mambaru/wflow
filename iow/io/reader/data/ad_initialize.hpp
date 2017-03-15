#pragma once

#include <iow/io/reader/data/tags.hpp>

namespace iow{ namespace io{ namespace reader{ namespace data{


struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, const O& opt)
  {
    t.get_aspect().template get<_read_buffer_>().set_options(opt);
  }
};


}}}}
