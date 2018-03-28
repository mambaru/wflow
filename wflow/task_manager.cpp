#include "task_manager.hpp"
#include "timer_manager.hpp"
#include "bique.hpp"
#include "thread_pool.hpp"
#include "asio.hpp"
#include <chrono>

namespace wflow{

  
class task_manager::pool_impl
  : public thread_pool<task_manager::queue_type>
{
  typedef thread_pool<task_manager::queue_type> super;
public:
  explicit pool_impl(const super::service_ptr& service)
    : super(service)
  {}
};


task_manager::task_manager( size_t queue_maxsize, size_t threads, bool use_asio )
  : _threads(threads)
  , _queue( std::make_shared<queue_type>(queue_maxsize, use_asio) )
  , _timer( std::make_shared<timer_manager<queue_type> >(_queue) )
  , _pool( std::make_shared<pool_type>(_queue) )
{
}
  
task_manager::task_manager( io_service_type& io, size_t queue_maxsize, size_t threads, bool use_asio /*= false*/  )
  : _threads(threads)
{
  _queue = std::make_shared<queue_type>(io, queue_maxsize, use_asio, threads!=0  );
  _timer = std::make_shared<timer_type>(_queue);
  _pool = std::make_shared<pool_type>(_queue);
}

void task_manager::reconfigure(size_t queue_maxsize, size_t threads, bool use_asio /*= false*/ )
{
  _threads = threads;
  _pool->reconfigure(_threads);
  _queue->reconfigure(queue_maxsize, use_asio, threads!=0 );
}
  
void task_manager::rate_limit(size_t rps) 
{
  _rate_limit = rps;
  /*if ( _pool!=nullptr) 
    _pool->rate_limit(rps);*/
}

void task_manager::set_startup( startup_handler handler )
{
  if ( _pool!=nullptr) 
    _pool->set_startup(handler);
}

void task_manager::set_finish( finish_handler handler )
{
  if ( _pool!=nullptr) 
    _pool->set_finish(handler);
}

void task_manager::set_statistics( statistics_handler handler )
{
  if ( _pool!=nullptr) 
    _pool->set_statistics(handler);
}


void task_manager::start()
{
  if ( _pool!=nullptr) 
  {
    _pool->start(_threads);
  }
}

void task_manager::stop()
{
  _timer->clear();
  _queue->stop();
  if ( _pool!=nullptr) 
    _pool->stop();
  _queue->reset();
}

std::size_t task_manager::run()
{
  return _queue->run();
}
  
std::size_t task_manager::run_one()
{
  return _queue->run_one();
}
  
std::size_t task_manager::poll_one()
{
  return _queue->poll_one();
}

void task_manager::safe_post( function_t f)
{
  _queue->safe_post(std::move(f));
}
  
void task_manager::safe_post_at(time_point_t tp, function_t f)
{
  _queue->safe_post_at( tp, std::move(f));
}

void task_manager::safe_delayed_post(duration_t duration, function_t f)
{
  _queue->safe_delayed_post(duration, std::move(f));
}
 
 
bool task_manager::post( function_t f, function_t drop )
{
  using namespace std::chrono;
  typedef time_point< steady_clock, nanoseconds > steady_point_t;
  time_t nanospan = 0;
  if ( _rate_limit == 0 )
  {
    return _queue->post(std::move(f) );
  }
  else if (_start_interval == 0)
  {
    _start_interval = duration_cast<nanoseconds>( steady_clock::now() - steady_point_t() ).count();
    ++_interval_count;
  }
  else
  {
    time_t now = duration_cast<nanoseconds>( steady_clock::now() - steady_point_t() ).count();
    nanospan = now - _start_interval;
    if ( nanospan > 1000000000  )
    {
      _start_interval = now;
      _interval_count = 1;
    }
    else
      ++_interval_count;
  }
  
  if ( _interval_count <= _rate_limit )
    return _queue->post(std::move(f) );
  else
    return this->delayed_post(
              nanoseconds((_interval_count*1000000000)/_rate_limit), 
              [this, f](){this->post(f);}
            );
}
  
bool task_manager::post_at(time_point_t tp, function_t f)
{
  return _queue->post_at( tp, std::move(f));
}

bool task_manager::delayed_post(duration_t duration, function_t f)
{
  return _queue->delayed_post(duration, std::move(f));
}
  
std::size_t task_manager::full_size() const
{
  return _queue->full_size();
}

std::size_t task_manager::safe_size() const
{
  return _queue->safe_size();
}

std::size_t task_manager::unsafe_size() const
{
  return _queue->unsafe_size();
}

std::size_t task_manager::dropped() const
{
  return _queue->dropped();
}
  
std::shared_ptr<task_manager::timer_type> task_manager::timer() const
{
  return _timer;
}

}
