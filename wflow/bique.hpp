#pragma once

#include <wflow/asio.hpp>
#include <functional>
#include <chrono>

namespace wflow{
  
class delayed_queue;
class asio_queue;
 
class bique
{
  typedef std::shared_ptr<delayed_queue> delayed_ptr;
  typedef std::shared_ptr<asio_queue> asio_ptr;
public:
  typedef ::wflow::asio::io_service io_service_type;
  typedef io_service_type::work work_type;
  typedef std::shared_ptr<io_service_type> io_service_ptr;
  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef time_point_t::duration                              duration_t;
  
  virtual ~bique();
 
  bique( size_t maxsize, bool use_asio);

  bique( io_service_type& io, size_t maxsize, bool use_asio, bool mt );
  
  void reconfigure( size_t maxsize, bool use_asio, bool mt );
  
  //void reconfigure(size_t maxsize, bool use_asio );

  void reset();

  std::size_t run();
  
  std::size_t run_one();
  
  std::size_t poll_one();

  void stop();

  bool post( function_t f, function_t drop );

  bool post_at(time_point_t tp, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);
  
  std::size_t size() const;
  
  std::size_t dropped() const;
  
  io_service_type::work work() const;
private:
  
  template<typename R, typename... Args>
  R invoke_( 
    R(delayed_queue::* method1)(Args...),
    R(asio_queue::* method2)(Args...), 
    Args... args);

  template<typename R, typename... Args>
  R invoke_( 
    R(delayed_queue::* method1)(Args...) const,
    R(asio_queue::* method2)(Args...) const, 
    Args... args) const;

private:
  std::atomic<bool> _dflag;
  std::atomic<bool> _mt_flag;
  io_service_ptr _io;
  delayed_ptr _delayed;
  asio_ptr   _asio;
  asio_ptr   _asio_st;
  
};

}
