#pragma once

#include <iow/io/acceptor/tags.hpp>
#include <iow/logger/logger.hpp>
#include <memory>
#include <iostream>

namespace iow{ namespace io{ namespace acceptor{

struct ad_confirm
{
  template<typename T, typename P>
  void operator()(T& t, P p)
  {
    const auto& context = t.get_aspect().template get<_context_>();
    if ( context.max_connections == 0 || context.manager->size() < static_cast<size_t>(context.max_connections)  )
    {
      p->start();
      context.manager->attach(p->get_id(), p);
    }
    else
    {
      p->stop();
    };
  }

};

}}}
