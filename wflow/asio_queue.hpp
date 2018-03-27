#pragma once

#include <wflow/asio.hpp>
#include <mutex>
#include <memory>
#include <atomic>
#include <chrono>

namespace wflow{
  
class asio_queue
  : public std::enable_shared_from_this<asio_queue >
{
  typedef asio_queue self;
  typedef ::wflow::asio::deadline_timer timer_type;
  typedef std::shared_ptr<timer_type> timer_ptr;
  typedef ::wflow::asio::io_service io_service_type;
  typedef std::mutex mutex_type;
public:
  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef time_point_t::duration                              duration_t;

  
  asio_queue( asio_queue const & ) = delete;
  void operator=( asio_queue const & ) = delete;

  asio_queue(io_service_type& io, size_t maxsize);
  
  void set_maxsize(size_t maxsize);

  std::size_t run();
  
  std::size_t run_one();
  
  std::size_t poll_one();
  
  void reset();
  
  void stop();

  void safe_post( function_t f );
 
  void safe_post_at(time_point_t tp, function_t f);
  
  void safe_delayed_post(duration_t duration, function_t f);
  
  bool post( function_t f);
 
  bool post_at(time_point_t tp, function_t f);
  
  bool delayed_post(duration_t duration, function_t f);

  std::size_t full_size() const;
  std::size_t unsafe_size() const;
  std::size_t safe_size() const;
  std::size_t dropped() const;
  
  io_service_type::work work() const;
  
private:
  
  bool check_();
  
  template<typename TP>
  timer_ptr create_timer_(TP tp);

private:
  io_service_type& _io;
  std::atomic<size_t> _counter;
  std::atomic<size_t> _safe_counter;
  std::atomic<size_t> _maxsize;
  std::atomic<size_t> _drop_count;
};

} // wflow

