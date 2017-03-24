#pragma once

#include <iow/io/socket/dgram/socket.hpp>
#include <iow/io/server/server.hpp>
#include <iow/ip/udp/server/aspect.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace ip{ namespace udp{ namespace server{

  /*
template <typename ConnectionType, typename A = fas::empty_type>
class acceptor
  : public ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect<ConnectionType> >::type >
{
  typedef ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect<ConnectionType> >::type > super;
public:
  typedef ConnectionType connection_type;
  typedef typename super::descriptor_type descriptor_type;
  typedef typename super::mutex_type mutex_type;

  acceptor(descriptor_type&& desc)
    : super( std::forward<descriptor_type>(desc))
  {}

  template<typename O>
  void listen(O&& opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    this->get_aspect().template get<_listen_>()(*this, std::forward<O>(opt) );
  }
};
*/

template <typename A = fas::empty_type>
class worker_base
  : public ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect >::type >
{
  typedef ::iow::io::descriptor::holder< typename fas::merge_aspect<A, aspect >::type > super;
public:
  typedef typename super::descriptor_type descriptor_type;
  typedef typename super::mutex_type mutex_type;

  worker_base(descriptor_type&& desc)
    : super( std::forward<descriptor_type>(desc))
  {}
  
  template<typename O>
  void listen(O&& opt)
  {
    std::lock_guard<mutex_type> lk( super::mutex() );
    this->get_aspect().template get<_open_>()(*this, std::forward<O>(opt) );
  }

};

class worker: public worker_base<>
{
public:
  typedef worker_base<> super;
  typedef typename super::descriptor_type descriptor_type;
  worker(descriptor_type&& desc)
    : super( std::forward<descriptor_type>(desc))
  {}
};

template<typename Worker = worker >
using server = ::iow::io::server::server<Worker>;

/*
template<typename Worker = worker<> >
using server = ::iow::io::server::server<Worker>;
*/

/*
template<typename A = fas::aspect<> >
using server_base =
  ::iow::io::socket::dgram::socket_base< typename fas::merge_aspect<A, aspect >::type >;

template<typename A = fas::aspect<> >
using server =
  ::iow::io::socket::dgram::socket< typename fas::merge_aspect<A, aspect >::type >;
*/

}}}}
