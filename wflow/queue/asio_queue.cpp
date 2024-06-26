#include "asio_queue.hpp"
#include <wflow/logger.hpp>
#include <wflow/system/system.hpp>
#include <wflow/system/boost.hpp>
#include <fas/utility/useless_cast.hpp>
#include <memory>

namespace wflow{

asio_queue::asio_queue(io_context_type& io, const size_t maxsize)
  : _io(io)
{
  _counter = 0;
  _safe_counter = 0;
  _drop_count = 0;
  _maxsize = maxsize;
}

asio_queue::io_context_type& asio_queue::get_io_context()
{
  return _io;
}

void asio_queue::set_maxsize(size_t maxsize)
{
  _maxsize = maxsize;
}

std::size_t asio_queue::run()
{
  return _io.run();
}

std::size_t asio_queue::run_one()
{
  return _io.run_one();
}

bool asio_queue::stopped() const
{
  return _io.stopped();
}

std::size_t asio_queue::run_one_for_ms(time_t ms)
{
  return _io.run_one_for(std::chrono::milliseconds(ms));
}

std::size_t asio_queue::run_for_ms(time_t ms)
{
  return _io.run_for(std::chrono::milliseconds(ms));
}

std::size_t asio_queue::poll_one()
{
  return _io.poll_one();
}

void asio_queue::reset()
{
  _io.restart();
}

void asio_queue::stop()
{
  _io.stop();
}

void asio_queue::safe_post( function_t f)
{
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_safe_counter;
  boost::asio::post( _io,  [wthis, f]()
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_safe_counter;
      if ( f!=nullptr )
        f();
    }
  });
}

bool asio_queue::post( function_t f, function_t drop )
{
  if ( !this->check_(std::move(drop)) )
    return false;
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_counter;
  boost::asio::post(_io, [wthis, f]()
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_counter;
      if ( f!=nullptr )
        f();
    }
  });
  return true;
}

void asio_queue::safe_post_at(time_point_t tp, function_t f)
{
  if ( tp <= time_point_t::clock::now() )
    return this->safe_post( std::move(f) );

  auto ptimer = this->create_timer_( tp );
  std::weak_ptr<self> wthis = this->shared_from_this();
  ++_safe_counter;
  ptimer->async_wait([f, ptimer, wthis]( const boost::system::error_code& )
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_safe_counter;
      if ( f!=nullptr )
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
  ptimer->async_wait([f, ptimer, wthis]( const boost::system::error_code& )
  {
    if (auto pthis = wthis.lock() )
    {
      --pthis->_counter;
      if ( f!=nullptr )
        f();
    }
  });
  return true;
}

void asio_queue::safe_delayed_post(duration_t duration, function_t f)
{
  this->safe_post_at( std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(duration), std::move(f));
}

bool asio_queue::delayed_post(duration_t duration, function_t f, function_t drop)
{
  return this->post_at( std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(duration), std::move(f), std::move(drop) );
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
    + ::boost::posix_time::seconds(fas::useless_cast<long>(sec))
    + ::boost::posix_time::microseconds(mksec);
  return std::make_shared<timer_type>( this->_io, ptime);
}

asio_queue::work_type asio_queue::work() const
{
  return work_type(_io.get_executor());
}

} // wflow

