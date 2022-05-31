#pragma once

#include <wflow/system/asio.hpp>
#include <mutex>
#include <memory>
#include <atomic>
#include <chrono>

namespace wflow{

class asio_queue
  : public std::enable_shared_from_this<asio_queue >
{
  typedef asio_queue self;
  typedef boost::asio::deadline_timer timer_type;
  typedef std::shared_ptr<timer_type> timer_ptr;
  typedef boost::asio::io_context io_context_type;
  typedef boost::asio::executor_work_guard<io_context_type::executor_type> work_type;
  typedef std::unique_ptr<work_type> work_ptr;
  typedef std::mutex mutex_type;
public:
  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration duration_t;


  asio_queue( asio_queue const & ) = delete;
  void operator=( asio_queue const & ) = delete;

  asio_queue(io_context_type& io, size_t maxsize);

  io_context_type& get_io_context();
  
  void set_maxsize(size_t maxsize);

  std::size_t run();

  std::size_t run_one();

  std::size_t poll_one();

  void reset();

  void stop();

  void safe_post( function_t f );

  void safe_post_at(time_point_t tp, function_t f);

  void safe_delayed_post(duration_t duration, function_t f);

  bool post( function_t f, function_t drop );

  bool post_at(time_point_t tp, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);

  std::size_t full_size() const;
  std::size_t unsafe_size() const;
  std::size_t safe_size() const;
  std::size_t dropped() const;

  work_type work() const;

private:

  bool check_(function_t drop);

  template<typename TP>
  timer_ptr create_timer_(TP tp);

private:
  io_context_type& _io;
  std::atomic<size_t> _counter;
  std::atomic<size_t> _safe_counter;
  std::atomic<size_t> _maxsize;
  std::atomic<size_t> _drop_count;

};

} // wflow

