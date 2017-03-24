#pragma once

#include <iow/io/reader/data/tags.hpp>
#include <iow/logger/logger.hpp>

#include <utility>
#include <memory>

namespace iow{ namespace io{ namespace reader{ namespace data{
  
//template<typename DataType>
struct ad_next
{
  template<typename T>
  std::pair<char*, size_t> operator()(T& t)
  {
    // Проверить размер 
    auto& buff = t.get_aspect().template get<_read_buffer_>();
    auto p = buff.next();

    if ( buff.overflow() )
    {
      buff.clear();
      IOW_LOG_ERROR("read buffer overflow");
      t.get_aspect().template get< ::iow::io::_stop_>()(t);
    }
    return std::move(p);
  }
};


}}}}
