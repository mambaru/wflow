#pragma once

#include <wflow/timer/timer_manager.hpp>
#include <wflow/system/asio.hpp>
#include <wflow/workflow_options.hpp>

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

  typedef std::function<void()> function_t;
  typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration duration_t;
  typedef boost::asio::io_context io_context_type;
  typedef timer_manager<queue_type> timer_manager_t;

  typedef std::function<void(std::thread::id)> startup_handler;
  typedef std::function<void(std::thread::id)> status_handler;
  typedef std::function<void(std::thread::id)> finish_handler;

  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler;

  explicit task_manager( const workflow_options& opt );

  task_manager( io_context_type& io, const workflow_options& opt );

  io_context_type& get_io_context();

  bool reconfigure( const workflow_options& opt );

  void rate_limit(size_t rps);
  void set_startup( startup_handler handler );
  void set_status( status_handler handler, time_t status_ms );
  void set_finish( finish_handler handler );
  void set_statistics( statistics_handler handler );

  void start();
  void stop();
  void reset();
  void reset_timers();
  void reset_queues();
  void shutdown();
  void wait();

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
  std::size_t reset_count() const;
  std::shared_ptr<timer_manager_t> get_timer_manager() const;

private:
  bool post_(function_t f, function_t drop);

private:
  std::string _id;
  std::atomic<size_t> _threads;
  std::atomic<bool> _can_reconfigured;
  std::shared_ptr<queue_type> _queue;
  std::shared_ptr<timer_manager_t> _timer_manager;
  std::shared_ptr<pool_type>  _pool;

  std::atomic<size_t> _rate_limit;
  std::atomic<time_t> _start_interval;
  std::atomic<size_t> _interval_count;

  std::atomic<bool> _quiet_mode;
  std::atomic<bool> _overflow_reset;
  std::shared_ptr< std::atomic<size_t> > _reset_count;
  std::atomic<time_t> _overflow_time;
  std::atomic<bool> _wait_reset;
};

}
