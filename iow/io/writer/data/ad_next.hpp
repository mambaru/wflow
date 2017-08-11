#pragma once


#include <iow/io/writer/data/tags.hpp>
#include <utility>

namespace iow{ namespace io{ namespace writer{ namespace data{
  
struct ad_next
{
  template<typename T>
  std::pair<const char*, size_t> operator()(T& t) const
  {
    return t.get_aspect().template get<_write_buffer_>().next();
  }
};


}}}}
