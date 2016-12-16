
#include "thread_pool_base.hpp"
#include <iow/workflow/delayed_queue.hpp>
#include <iow/workflow/bique.hpp>
#include <iow/workflow/asio_queue.hpp>
#include <sys/syscall.h>

namespace iow {

thread_pool_base::thread_pool_base()
  : _started(false)
{
  _rate_limit = 0;
}

void thread_pool_base::rate_limit(size_t rps) 
{
  _rate_limit = rps;
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

size_t thread_pool_base::get_size( ) const
{
  std::lock_guard< std::mutex > lk(_mutex);
  return this->_threads.size();
}
size_t thread_pool_base::get_counter( size_t thread) const
{
  std::lock_guard< std::mutex > lk(_mutex);
  if ( thread >= _counters.size() )
    return 0;
  return _counters[thread];
}

std::vector< int > thread_pool_base::get_ids() const
{
  std::lock_guard< std::mutex > lk(_mutex);
  return _threads_ids;
  
  /*std::vector< int > ids;
  ids.reserve( this->_threads.size() );
  for (auto& t : this->_threads)
  {
    ids.push_back( t.native_handle() );
  }
  return std::move( ids );
  */
}

// только после _service->stop();
void thread_pool_base::stop()
{
  std::lock_guard< std::mutex > lk(_mutex);

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
    for ( size_t i = threads; i < _threads.size(); ++i)
      _threads[i].detach();
    _threads.resize(threads);
    _flags.resize(threads);
    _counters.resize(threads);
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

void thread_pool_base::add_id(int id) 
{
  std::lock_guard<std::mutex> lk(_mutex);
  _threads_ids.push_back(id);
}

template<typename S>
void thread_pool_base::run_more_(std::shared_ptr<S> s, size_t threads)
{
  size_t prev_size = _threads.size();
  _threads.reserve( prev_size + threads);
  _counters.resize( prev_size + threads );
  for (size_t i = 0 ; i < threads; ++i)
  {
    thread_flag pflag = std::make_shared<bool>(true);
    std::weak_ptr<bool> wflag = pflag;
    std::weak_ptr<self> wthis = this->shared_from_this();
    _flags.push_back(pflag);
    auto& counter = _counters[prev_size + i];
    counter = 0;
    _threads.push_back( std::thread( [wthis, s, wflag, &counter]()
    {
      if ( auto pthis = wthis.lock() )
        pthis->add_id( syscall(SYS_gettid) );
      auto start = std::chrono::system_clock::now();
      size_t count = 0;
      while ( auto pthis = wthis.lock() )
      {
        size_t handlers = s->run_one();
        if ( handlers == 0 )
          break;
        if ( wflag.lock() == nullptr)
          break;
        counter += handlers;
        if ( pthis->_rate_limit != 0 )
        {
          count += handlers;
          if ( count >= pthis->_rate_limit )
          {
            auto now = std::chrono::system_clock::now();
            auto tm_ms = std::chrono::duration_cast< std::chrono::milliseconds >( now - start ).count();
            if ( tm_ms < 1000 )
              std::this_thread::sleep_for( std::chrono::milliseconds(1000-tm_ms)  );
            
            count = 0;
            start = std::chrono::system_clock::now();
          }
        }
      }
    }));
  }
}

}
