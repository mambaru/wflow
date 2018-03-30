#include "asio_queue.hpp"
#include <wflow/system/system.hpp>
#include <wflow/system/boost.hpp>
#include <memory>

namespace wflow{
  
asio_queue::asio_queue(io_service_type& io, const size_t maxsize)
  : _io(io)
{
  _counter = 0;
  _safe_counter = 0;
  _drop_count = 0;
  _maxsize = maxsize;
}
  
void asio_queue::set_maxsize(size_t maxsize)
{
  _maxsize = maxsize;
}

std::size_t asio_queue::run()
{
  ::wflow::system::error_code ec;
  return _io.run(ec);
}
  
std::size_t asio_queue::run_one()
{
  ::wflow::system::error_code ec;
  std::size_t count = _io.run_one(ec);
  return count;
}
  
std::size_t asio_queue::poll_one()
{
  ::wflow::system::error_code ec;
  return _io.poll_one(ec);
}

void asio_queue::reset() 
{
  _io.reset();
}

void asio_queue::stop()
{
  _io.stop();
}

void asio_queue::safe_post( function_t f)
{
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_safe_counter;
  _io.post( [wthis, f]()
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_safe_counter;
      f();
    }
  } );
}

bool asio_queue::post( function_t f, function_t drop )
{
  if ( !this->check_(std::move(drop)) )
    return false;  
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_counter;
  _io.post( [wthis, f]()
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_counter;
      f();
    }
  } );
  return true;
}

void asio_queue::safe_post_at(time_point_t tp, function_t f)
{
  if ( tp <= time_point_t::clock::now() )
    return this->safe_post( std::move(f) );

  auto ptimer = this->create_timer_( tp );
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_safe_counter;
  ptimer->async_wait([f, ptimer, wthis]( const ::wflow::system::error_code& )
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_safe_counter;
      f();
    }
  });
}

bool asio_queue::post_at(time_point_t tp, function_t f, function_t drop)
{
  if ( tp <= time_point_t::clock::now() )
    return this->post( std::move(f), std::move(drop) );

  if ( !this->check_(std::move(drop)) )
    return false;

  auto ptimer = this->create_timer_( tp );
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_counter;
  ptimer->async_wait([f, ptimer, wthis]( const ::wflow::system::error_code& )
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_counter;
      f();
    }
  });
  return true;
}

void asio_queue::safe_delayed_post(duration_t duration, function_t f)
{
  this->safe_post_at( std::chrono::system_clock::now() + duration, std::move(f));
}

bool asio_queue::delayed_post(duration_t duration, function_t f, function_t drop)
{
  return this->post_at( std::chrono::system_clock::now() + duration, std::move(f), std::move(drop) );
}

std::size_t asio_queue::unsafe_size() const
{
  return _counter;
}

std::size_t asio_queue::safe_size() const
{
  return _safe_counter;
}

std::size_t asio_queue::full_size() const
{
  return _safe_counter + _counter;
}

std::size_t asio_queue::dropped() const
{
  return _drop_count;
}
  
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
  
bool asio_queue::check_(function_t drop)
{
  if ( _maxsize == 0 )
    return true;
  if ( _counter < _maxsize )
    return true;
  ++_drop_count;
  if ( drop != nullptr)
    drop();
  return false;
}

template<typename TP>
asio_queue::timer_ptr asio_queue::create_timer_(TP tp)
{
  typedef std::chrono::microseconds microseconds; 
  typedef microseconds::rep rep_t; 
  rep_t d = std::chrono::duration_cast<microseconds>(tp.time_since_epoch()).count(); 
  rep_t sec = d/1000000; 
  rep_t mksec = d%1000000; 
  ::boost::posix_time::ptime ptime = 
    ::boost::posix_time::from_time_t(0)
    + ::boost::posix_time::seconds(static_cast<long>(sec))
    + ::boost::posix_time::microseconds(mksec);
  return std::make_shared<timer_type>( this->_io, ptime);
}

asio_queue::work_type asio_queue::work() const
{
  return work_type(_io);
  //return std::make_unique<work_type>(_io);
}

} // wflow

