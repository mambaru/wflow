#include "timer_manager_base.hpp"
#include "private/timer_handler.hpp"
#include "private/time_parser.hpp"

#include <wflow/queue/asio_queue.hpp>
#include <wflow/queue/delayed_queue.hpp>
#include <wflow/queue/bique.hpp>
#include <wflow/logger.hpp>


#include <iostream>

namespace wflow{

timer_manager_base::timer_manager_base()
  : _id_counter(0)
{}

std::shared_ptr<bool> timer_manager_base::detach(timer_id_t id)
{
  std::shared_ptr<bool> res;
  std::lock_guard< mutex_type > lk(_mutex);
  auto itr = _id_map.find(id);
  if ( itr == _id_map.end() )
    return nullptr;
  res = itr->second;
  _id_map.erase(itr);
  return res;
}

bool timer_manager_base::release( timer_id_t id )
{
  std::lock_guard< mutex_type > lk(_mutex);
  auto itr = _id_map.find(id);
  if ( itr == _id_map.end() )
    return false;
  _id_map.erase(itr);
  return true;
}

size_t timer_manager_base::reset()
{
  std::lock_guard< mutex_type > lk(_mutex);
  size_t s = _id_map.size();
  _id_map.clear();
  return s;
}

size_t timer_manager_base::size() const
{
  std::lock_guard< mutex_type > lk(_mutex);
  return _id_map.size();
}

template<typename Q, typename Handler>
timer_manager_base::timer_id_t
  timer_manager_base::create_( std::shared_ptr<Q> pq,  time_point_t start_time, duration_t delay, Handler h, expires_at expires)
{
  if ( delay.count() == 0 )
    delay = std::chrono::hours(24);
  timer_id_t id = ++_id_counter;
  std::shared_ptr<bool> pflag = std::make_shared<bool>(true);
  std::weak_ptr<bool> wflag = pflag;
  _id_map.insert( std::make_pair(id, pflag) );
  if ( start_time!=time_point_t() )
    pq->safe_post_at( start_time, timer_handler::make(pq, delay, std::move(h), expires, wflag) );
  else
    pq->safe_post( timer_handler::make(pq, delay, std::move(h), expires, wflag));
  return id;
}

template<typename Q, typename Handler>
timer_manager_base::timer_id_t
  timer_manager_base::create_( 
    std::shared_ptr<Q> pq, 
    const std::string& schedule, 
    Handler h, 
    expires_at expires
  )
{
  time_point_t tp = clock_t::now();
  duration_t delay = std::chrono::microseconds(0);
  std::string err;
  
  if ( time_parser::is_time(schedule) )
  {
    if ( time_parser::make_time_point( schedule, &tp, &err ) )
    { 
      return this->create_(pq, tp, std::chrono::microseconds(0), std::move(h), expires);                                                                
    }    
  }
  else if (time_parser::is_interval(schedule))
  {
    if ( time_parser::make_duration( schedule, &delay, &err ) )
    { 
      return this->create_(pq, tp, delay, std::move(h), expires);                                                                
    }    
  }
  else 
  {
    time_parser::cron_t crn;
    if ( time_parser::make_cron( schedule, &crn, &err ) )
    {
      std::time_t next = time_parser::cron_next(crn);
      timer_id_t id = ++_id_counter;
      std::shared_ptr<bool> pflag = std::make_shared<bool>(true);
      std::weak_ptr<bool> wflag = pflag;
      _id_map.insert( std::make_pair(id, pflag) );
      pq->safe_post_at( clock_t::from_time_t(next), timer_handler::make(pq, std::move(crn), std::move(h), expires, wflag) );
      return id;
    }
  }
  WFLOW_LOG_ERROR("Bad time-expression for timer '" << schedule << "':" << err);
  return -1;
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<bique> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<bique> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<delayed_queue> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<delayed_queue> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<asio_queue> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<asio_queue> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

///
///
///


timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<bique> pq, const std::string& schedule, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<bique> pq, const std::string& schedule, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<delayed_queue> pq, const std::string& schedule, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<delayed_queue> pq, const std::string& schedule, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<asio_queue> pq,  const std::string& schedule, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<asio_queue> pq, const std::string& schedule, async_handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}


}
