#pragma once

#include <iow/io/reader/data/tags.hpp>
#include <iow/logger.hpp>

#include <utility>
#include <memory>

namespace iow{ namespace io{ namespace reader{ namespace data{
  
struct ad_next
{
  template<typename T>
  std::pair<char*, size_t> operator()(T& t) const
  {
    // Проверить размер 
    auto& buf = t.get_aspect().template get<_read_buffer_>();
    auto p = buf.next();

    if ( buf.overflow() )
    {
      buf.clear();
      IOW_LOG_ERROR("Read buffer overflow. The descriptor will be closed.");
      t.get_aspect().template get< ::iow::io::_stop_>()(t);
    }
    else if ( p.first == nullptr )
    {
      buf.clear();
      IOW_LOG_ERROR("Read buffer. Out of memory.");
      t.get_aspect().template get< ::iow::io::_stop_>()(t);
    }
    return std::move(p);
  }
};


}}}}
