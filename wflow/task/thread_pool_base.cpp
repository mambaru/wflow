#include "thread_pool_base.hpp"

#include <wflow/queue/delayed_queue.hpp>
#include <wflow/queue/bique.hpp>
#include <wflow/queue/asio_queue.hpp>

#include <sys/syscall.h>
#include <sys/types.h>
#include <chrono>


namespace wflow {

thread_pool_base::thread_pool_base()
  : _started(false)
  , _rate_limit(0)
{
}

void thread_pool_base::rate_limit(size_t rps) 
{
  _rate_limit = rps;
}

void thread_pool_base::set_startup( startup_handler handler )
{
  _startup = handler;
}

void thread_pool_base::set_finish( finish_handler handler )
{
  _finish = handler;
}

void thread_pool_base::set_statistics( statistics_handler handler )
{
  _statistics = handler;
}

bool thread_pool_base::reconfigure(std::shared_ptr<bique> s, size_t threads) 
{
  return this->reconfigure_(s, threads); 
}

bool thread_pool_base::reconfigure(std::shared_ptr<asio_queue> s, size_t threads)
{
  return this->reconfigure_(s, threads); 
}

bool thread_pool_base::reconfigure(std::shared_ptr<delayed_queue> s, size_t threads) 
{
  return this->reconfigure_(s, threads); 
}

void thread_pool_base::start(std::shared_ptr<bique> s, size_t threads) 
{
  this->start_(s, threads); 
}

void thread_pool_base::start(std::shared_ptr<asio_queue> s, size_t threads)
{ 
  this->start_(s, threads); 
}

void thread_pool_base::start(std::shared_ptr<delayed_queue> s, size_t threads) 
{
  this->start_(s, threads); 
}

// только после _service->stop();
void thread_pool_base::stop()
{
  std::lock_guard< std::mutex > lk(_mutex);

  _flags.clear();
  _works.clear();

  for (auto& t : _threads)
    t.join();

  _threads.clear();

  _started = false;
}

template<typename S>
bool thread_pool_base::reconfigure_(std::shared_ptr<S> s, size_t threads)
{
  std::lock_guard< std::mutex > lk(_mutex);
  
  if ( !_started )
    return false;
  
  if ( threads == _threads.size() ) 
    return false;
  
  if ( threads > _threads.size() ) 
  {
    size_t diff = threads - _threads.size();
    this->run_more_(s, diff);
  }
  else
  {
    size_t oldsize = _threads.size();
    for ( size_t i = threads; i < _threads.size(); ++i)
      _threads[i].detach();
    _threads.resize(threads);
    _flags.resize(threads);
    _works.resize(threads);
    oldsize*=2;
    for (;oldsize!=0; --oldsize )
    {
      s->safe_post([oldsize](){ 
        std::this_thread::sleep_for( std::chrono::seconds(1) );
      });
    }
  }
  return true;
}

template<typename S>
void thread_pool_base::start_(std::shared_ptr<S> s, size_t threads)
{
  std::lock_guard< std::mutex > lk(_mutex);
  
  if ( _started )
    return;
  
  _started = true;
  
  if ( !_threads.empty())
    return;
  
  this->run_more_(s, threads);
}


/*
template<typename T>
static void nowarn(T&){}
*/
template<typename S>
void thread_pool_base::run_more_(std::shared_ptr<S> s, size_t threads)
{
  size_t prev_size = _threads.size();
  _threads.reserve( prev_size + threads);
  for (size_t i = 0 ; i < threads; ++i)
  {
    thread_flag pflag = std::make_shared<bool>(true);
    _flags.push_back(pflag);
    auto w = s->work();
    _works.push_back([w](){});
    _threads.push_back( this->create_thread_(s, pflag) );
  }
}

template<typename S>
std::thread thread_pool_base::create_thread_( std::shared_ptr<S> s, std::weak_ptr<bool> wflag )
{
  std::weak_ptr<self> wthis = this->shared_from_this();
  return std::thread([wthis, s, wflag](){
      thread_pool_base::startup_handler startup;
      thread_pool_base::finish_handler finish;
      thread_pool_base::statistics_handler statistics;

      if ( auto pthis = wthis.lock() )
      {
        startup = pthis->_startup;
        finish = pthis->_finish;
        statistics = pthis->_statistics;
      }
      std::thread::id thread_id = std::this_thread::get_id();
      auto pthis = wthis.lock();
      if ( startup != nullptr )
        startup(thread_id);
      
      size_t count = 0;
      for (;;)
      {
        auto start = std::chrono::steady_clock::now();
        size_t handlers = s->run_one();
        if (  handlers == 0 )
          break;
        if ( wflag.lock() == nullptr)
          break;

        if ( statistics != nullptr || pthis->_rate_limit != 0 )
        {
          auto now = std::chrono::steady_clock::now();
          auto span = now - start ;
          if ( statistics != nullptr )
            statistics( thread_id, handlers, span );

          if ( pthis->_rate_limit != 0 )
          {
            count += handlers;
            if ( count >= pthis->_rate_limit )
            {
              auto ts_now = std::chrono::steady_clock::now();
              auto tm_mks = std::chrono::duration_cast< std::chrono::microseconds >( ts_now - start ).count();
              if ( tm_mks < 1000000 )
                std::this_thread::sleep_for( std::chrono::microseconds(1000000-tm_mks)  );
              count = 0;
            }
          }
        }

      }
      if ( finish != nullptr )
        finish(thread_id);    
  });
}


}
