#pragma once

#include <iow/io/writer/data/tags.hpp>

namespace iow{ namespace io{ namespace writer{ namespace data{
  
struct ad_clear
{
  template<typename T>
  void operator()(T& t ) const
  {
    t.get_aspect().template get<_write_buffer_>().clear();
    //t.get_aspect().template get<_read_buffer_>().clear();
  }
};


}}}}
