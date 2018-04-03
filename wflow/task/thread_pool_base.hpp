#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <memory>
#include <functional>
#include <wflow/system/asio.hpp>

namespace wflow {

class bique;
class asio_queue;
class delayed_queue;

class thread_pool_base
  : public std::enable_shared_from_this< thread_pool_base >
{
public:
  typedef thread_pool_base self;
  typedef std::shared_ptr<bool> thread_flag;
/*  typedef ::wflow::asio::io_service io_service_type;
  typedef io_service_type::work work_type;
  typedef std::unique_ptr<work_type> work_ptr;
  */

  typedef std::vector<thread_flag> flag_list;
  typedef std::function<void(std::thread::id)> startup_handler;
  typedef std::function<void(std::thread::id)> finish_handler;
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler;
  thread_pool_base();
  
  void rate_limit(size_t rps);
  void set_startup( startup_handler handler );
  void set_finish( finish_handler handler );
  void set_statistics( statistics_handler handler );
  
  bool reconfigure(std::shared_ptr<bique> s, size_t threads);
  bool reconfigure(std::shared_ptr<asio_queue> s, size_t threads);
  bool reconfigure(std::shared_ptr<delayed_queue> s, size_t threads);

  void start(std::shared_ptr<bique> s, size_t threads);
  void start(std::shared_ptr<asio_queue> s, size_t threads);
  void start(std::shared_ptr<delayed_queue> s, size_t threads);

  void stop();

private: 
  void add_id(int id) ;

  template<typename S>
  bool reconfigure_(std::shared_ptr<S> s, size_t threads);

  template<typename S>
  void start_(std::shared_ptr<S> s, size_t threads);

  template<typename S>
  void run_more_(std::shared_ptr<S> s, size_t threads);
  
  template<typename S>
  std::thread create_thread_( std::shared_ptr<S> s, std::weak_ptr<bool> wflag );

private:
  bool _started;
  std::atomic<size_t> _rate_limit;
  mutable std::mutex _mutex;
  mutable std::vector< std::thread > _threads;
  std::vector< thread_flag > _flags;
  std::function<void()> _work;
  //std::vector< std::function<void()> > _works;
  
  startup_handler _startup;
  finish_handler _finish;
  statistics_handler _statistics;
 };

}
