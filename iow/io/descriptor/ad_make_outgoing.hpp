#pragma once

#include <iow/io/descriptor/tags.hpp>
#include <iow/logger/logger.hpp>
#include <memory>
#include <mutex>
namespace iow{ namespace io{ namespace descriptor{

struct ad_make_outgoing
{
  template<typename T>
  struct context
  {
    typedef typename T::aspect::template advice_cast< _context_>::type type;
    typedef typename type::outgoing_handler_type outgoing_handler_t;
  };

  template<typename T>
  auto operator()(T& t) 
    -> typename context<T>::outgoing_handler_t
  {
    typedef typename context<T>::type context_type;
    return t.get_aspect().template get<_context_>().outgoing_handler;
  }
};
  
}}}
