#pragma once

#include <iow/io/writer/data/tags.hpp>
#include <memory>

namespace iow{ namespace io{ namespace writer{ namespace data{

struct ad_rollback
{
  template<typename T, typename P>
  void operator()(T& t, P /*p*/) const
  {
    t.get_aspect().template get<_write_buffer_>().rollback();
  }
};

}}}}
