#pragma once
#include <iow/logger.hpp>

namespace iow{ namespace io{ namespace descriptor{ 

struct ad_close
{
  template<typename T>
  void operator()(T& t) const
  {
    try
    {
      if ( t.descriptor().is_open() )
      {
        t.descriptor().close();
      }
    }
    catch(...)
    {
      abort();
    }
  }
};

}}}
