#pragma once

#include <wflow/thread_pool_base.hpp>

namespace iow {

template<typename Service >
class thread_pool
  : public thread_pool_base
{
  typedef thread_pool_base super;
public:
  typedef thread_pool<Service> self;
  typedef Service service_type;
  typedef std::shared_ptr<service_type> service_ptr;

  explicit thread_pool(const service_ptr& service)
    : _service(service)
  {
  }
  
  bool reconfigure(size_t threads)
  {
    return super::reconfigure(_service, threads);
  }
    
  void start(size_t threads)
  {
    super::start(_service, threads);
  }

private:
  service_ptr _service;
};

}
