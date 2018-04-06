#include <wflow/task/task_manager.hpp>
#include <wflow/task/thread_pool.hpp>
#include <wflow/timer/timer_manager.hpp>
#include <wflow/queue/bique.hpp>

#include <wflow/system/asio.hpp>
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


task_manager::task_manager( const workflow_options& opt  )
  : _threads(opt.threads)
  , _can_reconfigured(opt.use_asio)
  , _queue( std::make_shared<queue_type>(opt.maxsize, opt.use_asio) )
  , _timer( std::make_shared<timer_type >(_queue) )
  , _pool( std::make_shared<pool_type>(_queue) )
  , _rate_limit(opt.rate_limit)
  , _start_interval(0)
  , _interval_count(0)
{
}
  
task_manager::task_manager( io_service_type& io, const workflow_options& opt  )
  : _threads(opt.threads)
  , _can_reconfigured(opt.use_asio)
  , _queue( std::make_shared<queue_type>(io, opt.maxsize, opt.use_asio, opt.threads!=0) )
  , _timer( std::make_shared<timer_type >(_queue) )
  , _pool( std::make_shared<pool_type>(_queue) )
  , _rate_limit(opt.rate_limit)
  , _start_interval(0)
  , _interval_count(0)
{
}

bool task_manager::reconfigure(const workflow_options& opt  )
{
  if ( !_can_reconfigured || !opt.use_asio )
    return false;
  _queue->reconfigure(opt.maxsize, opt.use_asio, opt.threads!=0 );
  _pool->reconfigure(opt.threads);
  /*if ( _threads == 0 )
    _queue->reset();*/
  _threads = opt.threads;
  return true;
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
  _queue->stop();
  if ( _pool!=nullptr) 
    _pool->stop();

}

void task_manager::reset()
{
  _timer->clear();
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
  _queue->safe_post(f);
}
  
void task_manager::safe_post_at(time_point_t tp, function_t f)
{
  _queue->safe_post_at( tp, f);
}

void task_manager::safe_delayed_post(duration_t duration, function_t f)
{
  _queue->safe_delayed_post(duration, f);
}

bool task_manager::post( function_t f, function_t drop )
{
  using namespace std::chrono;
  typedef time_point< steady_clock, nanoseconds > steady_point_t;
  time_t nanospan = 0;
  if ( _rate_limit == 0 )
  {
    return _queue->post(f, drop );
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
    return _queue->post(f, drop);
  
  this->safe_delayed_post(
    nanoseconds((_interval_count*1000000000)/_rate_limit), 
    [this, f, drop]()
    {
      this->post(f, drop);
    }
  );

  return true;
}
  
bool task_manager::post_at(time_point_t tp, function_t f, function_t drop)
{
  this->safe_post_at(tp, std::bind(&task_manager::post, this, f, drop) );
  /*
  this->safe_post_at(tp, [this, f, drop]()
  {
    this->post(f, drop);
    
  });
  */
  return true;
  //return _queue->post_at( tp, f, drop);
}

bool task_manager::delayed_post(duration_t duration, function_t f, function_t drop)
{
  this->safe_delayed_post(duration, std::bind(&task_manager::post, this, f, drop) );
  return true;
  /*
  auto pres = std::make_shared<bool>(true);
  std::weak_ptr<bool> wres = pres;
  this->safe_delayed_post(duration, [this, f, drop, wres]()
  {
    bool res = this->post(f, drop);
    if (auto pr = wres.lock() )
    {
      *pr = res;
    }
  });
  return *pres;*/

  /*return _queue->delayed_post(duration, f, drop);*/
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
