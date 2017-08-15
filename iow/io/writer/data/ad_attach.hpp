#pragma once

#include <iow/io/writer/data/tags.hpp>
#include <iow/logger/logger.hpp>
#include <memory>

namespace iow{ namespace io{ namespace writer{ namespace data{
  
struct ad_attach
{
  template<typename T, typename D>
  void operator()(T& t, D d)
  {
    auto &buf = t.get_aspect().template get<_write_buffer_>();
    buf.attach(std::move(d));
    if ( buf.overflow() )
    {
      IOW_LOG_ERROR("Write buffer overflow! The descriptor will be closed!");
      IOW_LOG_ERROR("\tyou lost " << buf.size() << " bytes from " << buf.count() << " buffers! ( capacity=" << buf.capacity() << ")");
      buf.clear();
      t.get_aspect().template get< ::iow::io::_stop_>()(t);
    }
  }
};


}}}}
