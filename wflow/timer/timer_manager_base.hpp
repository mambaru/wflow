#pragma once

#include <wflow/expires_at.hpp>
#include <functional>
#include <chrono>
#include <memory>
#include <mutex>
#include <map>

namespace wflow{

class bique;
class delayed_queue;
class asio_queue;

class timer_manager_base
{
public:
  typedef std::function<bool()> handler;
  typedef std::function<void(bool)> handler_callback;
  typedef std::function<void(handler_callback)> async_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef int timer_id_t;

  typedef std::mutex mutex_type;
  typedef std::weak_ptr<bool> wflag_type;

  timer_manager_base();

  std::shared_ptr<bool> detach(timer_id_t id);

  bool release( timer_id_t id );

  size_t reset();

  size_t size() const;

  timer_id_t create( std::shared_ptr<bique> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<bique> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<delayed_queue> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<delayed_queue> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<asio_queue> pq,  time_point_t start_time, duration_t delay, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<asio_queue> pq,  time_point_t start_time, duration_t delay, async_handler h, expires_at expires);

  ///
  /// CRON
  /// 

  timer_id_t create( std::shared_ptr<bique> pq,  const std::string& schedule, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<bique> pq,  const std::string& schedule, async_handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<delayed_queue> pq, const std::string& schedule, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<delayed_queue> pq, const std::string& schedule, async_handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<asio_queue> pq, const std::string& schedule, handler h, expires_at expires);

  timer_id_t create( std::shared_ptr<asio_queue> pq, const std::string& schedule, async_handler h, expires_at expires);
  
protected:

  template<typename Q, typename Handler>
  timer_id_t create_( std::shared_ptr<Q> pq, time_point_t start_time, duration_t delay, Handler h, expires_at expires);

  template<typename Q, typename Handler>
  timer_id_t create_( std::shared_ptr<Q> pq, const std::string& schedule, Handler h, expires_at expires);

private:
  mutable mutex_type _mutex;

  typedef std::map< timer_id_t, std::shared_ptr<bool> > id_map;

  timer_id_t _id_counter;
  id_map     _id_map;
};

}
