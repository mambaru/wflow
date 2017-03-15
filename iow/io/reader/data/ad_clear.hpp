#pragma once

#include <iow/io/reader/data/tags.hpp>

namespace iow{ namespace io{ namespace reader{ namespace data{
  
struct ad_clear
{
  template<typename T>
  void operator()(T& t )
  {
    t.get_aspect().template get<_read_buffer_>().clear();
  }
};


}}}}
