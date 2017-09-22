#pragma once

#include <iow/io/socket/dgram/socket.hpp>
#include <iow/io/server/server.hpp>
#include <iow/ip/udp/server/aspect.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{

template <typename A = fas::empty_type>
class worker_base
  : public ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect >::type >
{
  typedef ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect >::type > super;
public:
  typedef typename super::descriptor_type descriptor_type;
  typedef typename super::mutex_type mutex_type;

  explicit worker_base(descriptor_type&& desc)
    : super( std::forward<descriptor_type>(desc))
  {}
  
  template<typename O>
  void listen(O opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    this->get_aspect().template get<_open_>()(*this, opt );
  }

};

class worker: public worker_base<>
{
public:
  typedef worker_base<> super;
  typedef typename super::descriptor_type descriptor_type;
  explicit worker(descriptor_type&& desc)
    : super( std::forward<descriptor_type>(desc))
  {}
};

template<typename Worker = worker >
using server_base = ::iow::io::server::server<Worker>;

class server: public server_base<>
{
  typedef server_base<> super;
public:
  typedef super::io_service_type io_service_type; 
  explicit server(io_service_type& io)
    : super( io )
  {}
};


}}}}
