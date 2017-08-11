#pragma once

#include <iow/io/types.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <mutex>

namespace iow{ namespace io{ namespace socket{ namespace dgram{ namespace asio{


struct ad_make_output
{
  template<typename T>
  auto operator()(T& t) -> output_handler_t
  {
    auto pep = t.get_aspect().template get<_current_endpoint_>();
    std::weak_ptr<T> wthis = t.shared_from_this();
    return t.wrap([wthis, pep]( data_ptr d) 
    {
      if ( auto pthis = wthis.lock() )
      {
        std::lock_guard<typename T::mutex_type> lk( pthis->mutex() );
        pthis->get_aspect().template get<_current_endpoint_>() = pep;
        pthis->get_aspect().template get< ::iow::io::descriptor::_output_ >()( *pthis, std::move(d)  );
      }
    }, nullptr);
  }
};
  
}}}}}
