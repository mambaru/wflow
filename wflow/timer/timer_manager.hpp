#pragma once

#include <wflow/timer/requester.hpp>
#include <wflow/timer/timer_manager_base.hpp>

#include <string>
#include <memory>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <locale>
#include <chrono>
#include <mutex>

namespace wflow{

template<typename Queue>
class timer_manager
  : public timer_manager_base
{
  typedef timer_manager_base super;
public:
  typedef timer_manager<Queue> self;
  typedef Queue queue_type;
  typedef std::shared_ptr<queue_type> queue_ptr;
  typedef std::function<bool()> handler;
  typedef std::function<void(bool)> timer_callback;
  typedef std::function<void(timer_callback)> async_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef int timer_id_t;

  typedef std::mutex mutex_type;
  typedef std::weak_ptr<bool> wflag_type;
  typedef std::map< timer_id_t, std::shared_ptr<bool> > id_map;

  explicit timer_manager( queue_ptr queue );

  // timers
  
  timer_id_t create( duration_t delay, handler h, expires_at expires = expires_at::after);

  timer_id_t create( duration_t delay, async_handler h, expires_at expires = expires_at::after);

  timer_id_t create(duration_t start_delay, duration_t delay, handler h, expires_at expires = expires_at::after);

  timer_id_t create(duration_t start_delay, duration_t delay, async_handler h, expires_at expires = expires_at::after);

  timer_id_t create(time_point_t start_time, duration_t delay, handler h, expires_at expires = expires_at::after);

  timer_id_t create(time_point_t start_time, duration_t delay, async_handler h, expires_at expires = expires_at::after);

  timer_id_t create(const std::string& start_time, duration_t delay, handler h, expires_at expires  = expires_at::after);

  timer_id_t create(const std::string& start_time, duration_t delay, async_handler h, expires_at expires  = expires_at::after);

  /// Evry 24 hours or CRON
  
  timer_id_t create(const std::string& schedule, handler h, expires_at expires = expires_at::after);

  timer_id_t create(const std::string& schedule, async_handler h, expires_at expires = expires_at::after);

  /// CRON
  
//  timer_id_t create_cron(std::string cron, handler h, expires_at expires = expires_at::after);

//  timer_id_t create_cron(std::string cron, async_handler h, expires_at expires = expires_at::after);

  // reqesters 

  template< typename Req, typename Res>
  timer_id_t create( 
    duration_t d, 
    typename requester::sender_t<Req, Res>::type sender, 
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->create( std::chrono::milliseconds(0), d, this->make_reqester_<Req, Res>( sender, generator));
  }

  template< typename Req, typename Res>
  timer_id_t create( 
    duration_t sd, 
    duration_t d, 
    typename requester::sender_t<Req, Res>::type sender, 
    typename requester::generator_t<Req, Res>::type generator 
  )
  {
    return this->create(sd, d, this->make_reqester_<Req, Res>(sender, generator));
  }

  template< typename Req, typename Res >
  timer_id_t create(
    time_point_t st,
    duration_t d, 
    typename requester::sender_t<Req, Res>::type sender, 
    typename requester::generator_t<Req, Res>::type generator)
  {
    return this->create(st, d, this->make_reqester_<Req, Res>(sender, generator));
  }

  template< typename Req, typename Res>
  timer_id_t create( 
    std::string st, 
    duration_t d, 
    typename requester::sender_t<Req, Res>::type sender, 
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->create(st, d, this->make_reqester_<Req, Res>(sender, generator));
  }

  template< typename Req, typename Res >
  timer_id_t create( 
    std::string st, 
    typename requester::sender_t<Req, Res>::type sender, 
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->create(st, this->make_reqester_<Req, Res>(sender, generator));
  }  

  /// //////////////

private:

  template<typename Req, typename Res>
  auto make_reqester_(
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  ) -> std::function<void(timer_callback)>
  {
    return requester::make<Req, Res>( this->_queue, sender,  generator);
  }

  template<typename Handler>
  timer_id_t create_(time_point_t start_time, duration_t delay, Handler h, expires_at expires);

  template<typename Handler>
  timer_id_t create_(const std::string& start_time, duration_t delay, Handler h, expires_at expires);

private:

  queue_ptr  _queue;
};

}
