#include "timer_manager_base.hpp"
#include "private/timer_handler.hpp"
#include "private/time_parser.hpp"

#include <wflow/queue/asio_queue.hpp>
#include <wflow/queue/native_queue.hpp>
#include <wflow/queue/bique.hpp>
#include <wflow/logger.hpp>

#include <vector>

namespace wflow{

timer_manager_base::timer_manager_base()
  : _id_counter(0)
{}

void timer_manager_base::erase_timer_(timer_id_t id)
{
  _id_map.erase(id);
  _rearms.erase(id);
}

std::shared_ptr<bool> timer_manager_base::detach(timer_id_t id)
{
  std::shared_ptr<bool> res;
  std::lock_guard< mutex_type > lk(_mutex);
  auto itr = _id_map.find(id);
  if ( itr == _id_map.end() )
    return res;
  res = itr->second;
  this->erase_timer_(id);
  return res;
}

bool timer_manager_base::release( timer_id_t id )
{
  std::lock_guard< mutex_type > lk(_mutex);
  auto itr = _id_map.find(id);
  if ( itr == _id_map.end() )
    return false;
  this->erase_timer_(id);
  return true;
}

size_t timer_manager_base::reset()
{
  std::lock_guard< mutex_type > lk(_mutex);
  size_t s = _id_map.size();
  _id_map.clear();
  _rearms.clear();
  return s;
}

size_t timer_manager_base::size() const
{
  std::lock_guard< mutex_type > lk(_mutex);
  return _id_map.size();
}

void timer_manager_base::rebind_flags()
{
  std::lock_guard< mutex_type > lk(_mutex);
  for (auto& kv : _id_map)
    kv.second = std::make_shared<bool>(true);
}

size_t timer_manager_base::rearm_all()
{
  std::vector<rearm_fun> jobs;
  {
    std::lock_guard< mutex_type > lk(_mutex);
    jobs.reserve(_rearms.size());
    for (auto& kv : _rearms)
    {
      if ( _id_map.find(kv.first) != _id_map.end() && kv.second )
        jobs.push_back(kv.second);
    }
  }
  for (const auto& job : jobs)
    job();
  return jobs.size();
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

  Handler h_sched = h;
  _rearms[id] = [this, id, pq, delay, h, expires]()
  {
    std::shared_ptr<bool> flag;
    {
      std::lock_guard< mutex_type > lk(_mutex);
      auto itr = _id_map.find(id);
      if ( itr == _id_map.end() )
        return;
      flag = itr->second;
    }
    Handler hc = h;
    pq->safe_post( timer_handler::make(pq, delay, std::move(hc), expires, flag) );
  };

  if ( start_time!=time_point_t() )
    pq->safe_post_at( start_time, timer_handler::make(pq, delay, std::move(h_sched), expires, wflag) );
  else
    pq->safe_post( timer_handler::make(pq, delay, std::move(h_sched), expires, wflag));
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

      time_parser::cron_t crn_sched = crn;
      Handler h_sched = h;
      _rearms[id] = [this, id, pq, crn, h, expires]()
      {
        std::shared_ptr<bool> flag;
        {
          std::lock_guard< mutex_type > lk(_mutex);
          auto itr = _id_map.find(id);
          if ( itr == _id_map.end() )
            return;
          flag = itr->second;
        }
        time_parser::cron_t crn_copy = crn;
        Handler hc = h;
        std::time_t n = time_parser::cron_next(crn_copy);
        pq->safe_post_at( clock_t::from_time_t(n), timer_handler::make(pq, std::move(crn_copy), std::move(hc), expires, flag) );
      };

      pq->safe_post_at( clock_t::from_time_t(next), timer_handler::make(pq, std::move(crn_sched), std::move(h_sched), expires, wflag) );
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

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<native_queue> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, start_time, delay, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<native_queue> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires)
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

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<native_queue> pq, const std::string& schedule, handler h, expires_at expires)
{
  std::lock_guard< mutex_type > lk(_mutex);
  return this->create_(pq, schedule, std::move(h), expires);
}

timer_manager_base::timer_id_t timer_manager_base::create( std::shared_ptr<native_queue> pq, const std::string& schedule, async_handler h, expires_at expires)
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
