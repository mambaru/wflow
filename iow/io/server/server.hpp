#pragma once

#include <iow/io/descriptor/mtdup.hpp>

namespace iow{ namespace io{ namespace server{

template<typename Acceptor>
using server_base = ::iow::io::descriptor::mtdup<Acceptor>;

template<typename Acceptor>
class server
  : private server_base<Acceptor>
{
  typedef server_base<Acceptor> super;
  
public: 
  typedef typename super::io_service_type io_service_type;
  typedef Acceptor acceptor_type;
  typedef std::shared_ptr<acceptor_type> acceptor_ptr;
  typedef typename acceptor_type::descriptor_type descriptor_type;
  
  explicit server(io_service_type& io)
    : super( std::move( descriptor_type(io) ) )
  {}
  
  template<typename Opt>
  // cppcheck-suppress functionStatic
  void start(Opt opt)
  {
    super::origin()->listen(opt);
    super::start(opt);
  }

  // cppcheck-suppress functionStatic
  void stop()
  {
    super::stop();
  }
};
  
}}}
