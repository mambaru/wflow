#pragma once

#include <wflow/timer/timer_manager.hpp>
#include <wflow/system/asio.hpp>
#include <thread>
#include <atomic>

namespace wflow{

class bique;

class task_manager
{
  class pool_impl;
  typedef pool_impl pool_type;
public:
  typedef bique queue_type;

  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef time_point_t::duration                              duration_t;
  typedef ::wflow::asio::io_service io_service_type;
  typedef timer_manager<queue_type> timer_type;
  
  typedef std::function<void(std::thread::id)> startup_handler;
  typedef std::function<void(std::thread::id)> finish_handler;
  
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler;


  //task_manager( io_service_type& io, size_t queue_maxsize );
  
  task_manager( size_t queue_maxsize, size_t threads, bool use_asio );
  
  task_manager( io_service_type& io, size_t queue_maxsize, size_t threads, bool use_asio /*= false*/  );

  void reconfigure(size_t queue_maxsize, size_t threads, bool use_asio /*= false*/ );
  
  void rate_limit(size_t rps);
  void set_startup( startup_handler handler );
  void set_finish( finish_handler handler );
  void set_statistics( statistics_handler handler );

  void start();
  void stop();
  void reset();

  std::size_t run();
  
  std::size_t run_one();
  
  std::size_t poll_one();
  
  void safe_post( function_t f);
  
  void safe_post_at(time_point_t tp, function_t f);

  void safe_delayed_post(duration_t duration, function_t f);
  
  bool post(function_t f, function_t drop);
  
  bool post_at(time_point_t tp, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);
  
  std::size_t full_size() const;
  std::size_t safe_size() const;
  std::size_t unsafe_size() const;
  std::size_t dropped() const;
  std::shared_ptr<timer_type> timer() const;
  
  
private:
  std::atomic<size_t> _threads;
  std::shared_ptr<queue_type> _queue;
  std::shared_ptr<timer_type> _timer;
  std::shared_ptr<pool_type>  _pool;
  
  std::atomic<size_t> _rate_limit;
  std::atomic<time_t> _start_interval;
  std::atomic<size_t> _interval_count;
  
};

}
