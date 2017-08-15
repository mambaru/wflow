#pragma once

#include <iow/io/client/multi_base.hpp>
#include <iow/logger/logger.hpp>
#include <mutex>
#include <memory>
#include <vector>

namespace iow{ namespace io{ namespace client{

template<typename Client>
class multi_thread: public multi_base<Client>
{
  using super = multi_base<Client>;
  //typedef multi_base<Client> super;

public:
  typedef typename super::io_service_type io_service_type;
  
  explicit multi_thread(io_service_type& io)
    : super(io)
  {}
  
  template<typename Opt>
  // cppcheck-suppress functionStatic
  void start(Opt opt)
  {
    super::start(std::move(opt), threads_(opt) );
  }

  template<typename Opt>
  // cppcheck-suppress functionStatic
  void reconfigure(Opt opt, size_t )
  {
    super::reconfigure(std::move(opt), threads_(opt) );
  }
  
private: 
  
  template<typename Opt>
  static size_t threads_(Opt& opt) 
  {
    return opt.threads==0 ? 1 : opt.threads;
  }
};

}}}
