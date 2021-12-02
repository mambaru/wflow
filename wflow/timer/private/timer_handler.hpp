#pragma once

#include <chrono>
#include <memory>
#include <functional>
#include <wflow/expires_at.hpp>
#include "croncpp/croncpp.h"


namespace wflow{

class delayed_queue;
class asio_queue;
class bique;

class timer_handler
{
public:
  typedef std::weak_ptr<bool> wflag_type;

  typedef std::function<bool()> handler;
  typedef std::function<void(bool)> handler_callback;
  typedef std::function<void(handler_callback)> async_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef cron::cronexpr cron_t;


  static std::function<void()> make( std::shared_ptr<bique>         pq, duration_t delay, handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<asio_queue>    pq, duration_t delay, handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<delayed_queue> pq, duration_t delay, handler h, expires_at expires, wflag_type wflag );

  static std::function<void()> make( std::shared_ptr<bique> pq,        duration_t delay,  async_handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<asio_queue> pq,   duration_t delay,  async_handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<delayed_queue> pq, duration_t delay, async_handler h, expires_at expires, wflag_type wflag );

///
  
  static std::function<void()> make( std::shared_ptr<bique>         pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<asio_queue>    pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<delayed_queue> pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<bique> pq,        const cron_t& delay,  async_handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<asio_queue> pq,   const cron_t& delay,  async_handler h, expires_at expires, wflag_type wflag );
  static std::function<void()> make( std::shared_ptr<delayed_queue> pq, const cron_t& delay, async_handler h, expires_at expires, wflag_type wflag );

};

}
