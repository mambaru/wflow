#pragma once

#include <iow/io/socket/stream/asio/ad_async_read_some.hpp>
#include <iow/io/socket/stream/asio/ad_async_write_some.hpp>
#include <iow/io/reader/tags.hpp>
#include <iow/io/writer/tags.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace io{ namespace socket{ namespace stream{ namespace asio{

  
typedef fas::aspect<
  fas::advice< ::iow::io::reader::_read_some_, ad_async_read_some>,
  fas::advice< ::iow::io::writer::_write_some_, ad_async_write_some>
> aspect;
  
}}}}}
