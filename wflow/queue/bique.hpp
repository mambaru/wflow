#pragma once

#include <wflow/system/asio.hpp>
#include <functional>
#include <chrono>
#include <atomic>

namespace wflow{

class delayed_queue;
class asio_queue;

class bique
{
  typedef std::shared_ptr<delayed_queue> delayed_ptr;
  typedef std::shared_ptr<asio_queue> asio_ptr;
public:
  typedef boost::asio::io_context io_context_type;
  typedef boost::asio::executor_work_guard<io_context_type::executor_type> work_type;
  typedef std::unique_ptr<work_type> work_ptr;
  typedef std::shared_ptr<io_context_type> io_context_ptr;
  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration duration_t;

  virtual ~bique();

  bique( size_t maxsize, bool use_asio);

  bique( io_context_type& io, size_t maxsize, bool use_asio, bool mt );

  io_context_type& get_io_context();

  void reconfigure( size_t maxsize, bool use_asio, bool mt );

  void reset();

  std::size_t run();

  std::size_t run_one();

  std::size_t poll_one();

  std::size_t run_one_for_ms(time_t ms);

  std::size_t run_for_ms(time_t ms);

  bool stopped() const;

  void stop();

  void safe_post( function_t f);

  void safe_post_at(time_point_t tp, function_t f);

  void safe_delayed_post(duration_t duration, function_t f);

  bool post( function_t f, function_t drop );

  bool post_at(time_point_t tp, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);

  std::size_t full_size() const;
  std::size_t safe_size() const;
  std::size_t unsafe_size() const;

  std::size_t dropped() const;

  work_type work() const;
private:

  template<typename R, typename... Args>
  R invoke_(
    R(delayed_queue::* method1)(Args...),
    R(asio_queue::* method2)(Args...),
    Args&&... args);

  template<typename R, typename... Args>
  R invoke_(
    R(delayed_queue::* method1)(Args...) const,
    R(asio_queue::* method2)(Args...) const,
    Args&&... args) const;

private:
  std::atomic<bool> _dflag;
  std::atomic<bool> _mt_flag;
  io_context_ptr _io;
  delayed_ptr _delayed;
  asio_ptr   _asio;
  asio_ptr   _asio_st;

};

}
