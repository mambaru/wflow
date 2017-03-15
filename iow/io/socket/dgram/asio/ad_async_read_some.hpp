#pragma once

#include <iow/asio.hpp>
#include <utility>

namespace iow{ namespace io{ namespace socket{ namespace dgram{ namespace asio{

struct ad_async_read_some/*ad_receive_from*/
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H&& handler)
  {
    t.descriptor().async_read_some(
      ::iow::asio::buffer( p.first, p.second ),
      std::forward<H>(handler)
    );

/*
    auto dd = std::make_shared<typename T::data_ptr>( std::move(d) );
    
    auto pep = std::make_shared<endpoint_type>();
    auto pthis = t.shared_from_this();
    
    auto callback = [pthis, dd, pep]( boost::system::error_code ec , std::size_t bytes_transferred )
    { 
      typename T::lock_guard lk(pthis->mutex());
      pthis->get_aspect().template get<_remote_endpoint_>() = std::move(*pep);
      pthis->get_aspect().template get<_read_handler_>()(*pthis, std::move(*dd), std::move(ec), bytes_transferred);
    };
    
    t.mutex().unlock();
    t.descriptor().async_receive_from( ::boost::asio::buffer( **dd ), *pep, callback);
    t.mutex().lock();
  */
  }
};

}}}}}
