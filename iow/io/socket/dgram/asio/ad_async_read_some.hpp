#pragma once

#include <iow/asio.hpp>
#include <iow/io/socket/dgram/tags.hpp>
#include <utility>

namespace iow{ namespace io{ namespace socket{ namespace dgram{ namespace asio{

struct ad_async_read_some/*ad_receive_from*/
{
  template<typename T, typename P, typename H>
  void operator()(T& t, P p, H&& handler)
  {
    using namespace std::placeholders;
    using endpoint_ptr = typename T::aspect::template advice_cast<_current_endpoint_>::type;
    using endpoint_type = typename endpoint_ptr::element_type;
    endpoint_ptr pep = std::make_shared<endpoint_type>();
    
    auto set_ep = [&t, pep](boost::system::error_code, size_t)
    {
      t.get_aspect().template get<_current_endpoint_>() = pep;
    };
    
    t.descriptor().async_receive_from( 
      ::iow::asio::buffer( p.first, p.second ),
      *pep, 
      std::bind(handler, _1, _2, set_ep)
    );
  }
};

}}}}}
