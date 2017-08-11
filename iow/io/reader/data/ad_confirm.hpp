#pragma once

#include <iow/io/reader/data/tags.hpp>
#include <iow/logger/logger.hpp>
#include <memory>

namespace iow{ namespace io{ namespace reader{ namespace data{

struct ad_confirm
{
  template<typename T, typename P>
  void operator()(T& t, P p) const
  {
    bool result = t.get_aspect().template get<_read_buffer_>().confirm( std::move(p) );
    if ( !result )
    {
      IOW_LOG_ERROR("iow::io::reader::data::ad_confirm: ошибка буфера")
      // read error
    }
  }
};

}}}}
