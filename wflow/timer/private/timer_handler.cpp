#include "timer_handler.hpp"
#include "basic_timer_handler.hpp"

#include <wflow/queue/bique.hpp>
#include <wflow/queue/asio_queue.hpp>
#include <wflow/queue/delayed_queue.hpp>


namespace wflow{

std::function<void()> timer_handler::make( std::shared_ptr<bique> pq,        duration_t delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<asio_queue> pq,   duration_t delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<delayed_queue> pq, duration_t delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<bique> pq,        duration_t delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<asio_queue> pq,   duration_t delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<delayed_queue> pq, duration_t delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

///
/// CRON
///

std::function<void()> timer_handler::make( std::shared_ptr<bique> pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<asio_queue> pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<delayed_queue> pq, const cron_t& delay, handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<bique> pq, const cron_t& delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<asio_queue> pq, const cron_t& delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}

std::function<void()> timer_handler::make( std::shared_ptr<delayed_queue> pq, const cron_t& delay, async_handler h, expires_at expires, wflag_type wflag )
{
  return basic_timer_handler::make(pq, delay, std::move(h), expires, wflag);
}


}
