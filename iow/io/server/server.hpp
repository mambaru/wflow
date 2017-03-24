#pragma once

#include <iow/io/descriptor/mtdup.hpp>

namespace iow{ namespace io{ namespace server{
  
template<typename Acceptor>
class server
  : private ::iow::io::descriptor::mtdup<Acceptor>
{
  typedef ::iow::io::descriptor::mtdup<Acceptor> super;
  
public: 
  typedef typename super::io_service_type io_service_type;
  typedef Acceptor acceptor_type;
  typedef std::shared_ptr<acceptor_type> acceptor_ptr;
  typedef typename acceptor_type::descriptor_type descriptor_type;
  
  server(io_service_type& io)
    : super( std::move( descriptor_type(io) ) )
  {}
  
  template<typename Opt>
  void start(Opt&& opt)
  {
    std::cout << "\n--3.1--\n";
    super::origin()->listen(opt);
    std::cout << "\n--3.2--\n";
    super::start(opt);
    std::cout << "\n--3.3--\n";
  }

  void stop()
  {
    super::stop();
  }
};
  
}}}
