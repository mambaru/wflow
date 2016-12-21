#include "delayed_queue.hpp"

namespace iow {

delayed_queue::delayed_queue(size_t maxsize)
  : _maxsize(maxsize)
{
  _drop_count = 0;
}

delayed_queue::~delayed_queue ()
{
  this->stop();
}
  
void delayed_queue::set_maxsize(size_t maxsize)
{
  std::unique_lock<mutex_t> lck( _mutex );
  _maxsize = maxsize;
}

void delayed_queue::reset()
{
  _drop_count = 0;
  _loop_exit = false;
}

std::size_t delayed_queue::run()
{
  std::unique_lock<mutex_t> lck( _mutex );
  if ( _loop_exit ) 
    return 0;
  lck.unlock();
  return this->loop_(lck, false);
}
  
std::size_t delayed_queue::run_one()
{
  if ( _loop_exit ) 
    return 0;
  std::unique_lock<mutex_t> lck( _mutex, std::defer_lock );
  return this->run_one_( lck );
}

std::size_t delayed_queue::poll_one()
{
  std::unique_lock<mutex_t> lck( _mutex, std::defer_lock );
  if ( _loop_exit ) 
    return 0;
  return this->poll_one_( lck );
}

void delayed_queue::stop()
{
  std::unique_lock<mutex_t> lck( _mutex );
  if ( !_loop_exit )
  { 
    _drop_count = 0;
    _loop_exit = true;
    _cond_var.notify_all();
    while ( !_que.empty() ) 
      _que.pop();
    while ( !_delayed_que.empty() ) 
      _delayed_que.pop();
  }
}
  
bool delayed_queue::post( function_t f )
{
  std::lock_guard<mutex_t> lock( _mutex );
  if ( !this->check_() )
    return false;

  _que.push( std::move( f ) );
  _cond_var.notify_one();
  return true;
}
  
bool delayed_queue::post_at(time_point_t time_point, function_t f)
{
  std::lock_guard<mutex_t> lock( _mutex );
  if ( !this->check_() )
    return false;

  this->push_at_( std::move(time_point), std::move(f) ); 
  _cond_var.notify_one();
  return true;
}

bool delayed_queue::delayed_post(duration_t duration, function_t f)
{  
  std::lock_guard<mutex_t> lock( _mutex );
  if ( !this->check_() )
    return false;

  if ( ! duration.count() )
    _que.push( std::move( f ) );
  else
    this->push_at_( std::move( std::chrono::system_clock::now() + duration), std::move(f)  );
  _cond_var.notify_one();
  return true;
}
  
std::size_t delayed_queue::size() const
{
  std::lock_guard<mutex_t> lck( _mutex );
  return this->size_();
}

std::size_t delayed_queue::dropped() const
{
  std::lock_guard<mutex_t> lck( _mutex );
  return _drop_count;
}

//private:
bool delayed_queue::check_()
{
  if ( _maxsize == 0 )
    return true;
  if ( this->size_() < _maxsize )
    return true;
  ++_drop_count;
  return false;
}

std::size_t delayed_queue::size_() const
{
  return _que.size() + _delayed_que.size();
}
void delayed_queue::push_at_(time_point_t time_point, function_t f)
{
  _delayed_que.emplace( time_point, std::move( f ) );
}
  
std::size_t delayed_queue::poll_one_( std::unique_lock<mutex_t>& lck)
{
  lck.lock();
  if ( ! _delayed_que.empty() )
  {
    if ( _delayed_que.top().first <= std::chrono::system_clock::now() )
    {
      _que.push( std::move( _delayed_que.top().second ) );
      _delayed_que.pop();
    }
  }
  if ( _que.empty() )
  {
    lck.unlock();
    return 0;
  }
  function_t run_func = std::move( _que.front() );
  _que.pop();
  lck.unlock();
   
  run_func();
    
  return 1;
}

std::size_t delayed_queue::run_one_( std::unique_lock<mutex_t>& lck)
{
  return this->loop_( lck, true);
}

std::size_t delayed_queue::loop_(std::unique_lock<mutex_t>& lck, bool one)
{
  std::size_t result = 0;
  while ( !_loop_exit )
  {
    if ( !this->poll_one_( lck ) )
    {
      this->run_wait_(lck);
    } 
    else if ( one )
    {
      return 1;
    }
    else
      ++result;
    
  }
  return result;
}

void delayed_queue::run_wait_( std::unique_lock<mutex_t> & lck)
{
  lck.lock();
  if ( _que.empty() && _delayed_que.empty() )
  {
    _cond_var.wait( lck );
  }
  else if ( _que.empty() && !_delayed_que.empty() )
  {
    _cond_var.wait_until( lck, _delayed_que.top().first );
  }
  lck.unlock();
}

}


