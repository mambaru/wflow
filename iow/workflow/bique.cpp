#include "bique.hpp"
#include "asio_queue.hpp"
#include "delayed_queue.hpp"

namespace iow{
 
bique::~bique()
{
  this->stop();
}

bique::bique( size_t maxsize, bool use_asio)
{
  _dflag = !use_asio;
  _io = std::make_shared<io_service_type>();
  _asio = std::make_shared<asio_queue>( *_io, maxsize);
  _delayed = std::make_shared<delayed_queue>(maxsize);
  _asio_st = 0;
  _mt_flag = true;
}

bique::bique( io_service_type& io, size_t maxsize, bool use_asio, bool mt )
{
  _dflag = !use_asio;
  _mt_flag = mt;
  _io = std::make_shared<io_service_type>();
  _asio = std::make_shared<asio_queue>( *_io, maxsize);
  _asio_st = std::make_shared<asio_queue>( io, maxsize);
  /*
  _dflag = !use_asio;
  _io = std::make_shared<io_service_type>();
  if ( mt )
    _asio = std::make_shared<asio_queue>( *_io, maxsize);
  else
    _asio = std::make_shared<asio_queue>( io, maxsize);
  _delayed = std::make_shared<delayed_queue>(maxsize);
  */
}

/*
bique::bique( size_t maxsize )
{
  _dflag = true;
  _delayed = std::make_shared<delayed_queue>(maxsize);
  _asio  = nullptr;
}

bique::bique( io_service_type& io, size_t maxsize, bool use_asio  )
{
  _dflag = !use_asio;
  _delayed = std::make_shared<delayed_queue>(maxsize);
  _asio = std::make_shared<asio_queue>(io, maxsize);
}
*/


void bique::reconfigure(size_t maxsize, bool use_asio, bool mt )
{
  /*
  bool newflag = !use_asio;
  if ( _asio == nullptr ) 
    newflag = true;
  _dflag = newflag;
  */
  _dflag = use_asio;
  _mt_flag = mt;
  _delayed->set_maxsize(maxsize);
  _asio->set_maxsize(maxsize);
  if( _asio_st ) 
    _asio->set_maxsize(maxsize);
}

void bique::reset()
{
  return this->invoke_( &delayed_queue::reset, &asio_queue::reset);
}

std::size_t bique::run()
{
  return this->invoke_( &delayed_queue::run, &asio_queue::run);
}
  
std::size_t bique::run_one()
{
  return this->invoke_( &delayed_queue::run_one, &asio_queue::run_one);
}
  
std::size_t bique::poll_one()
{
  return this->invoke_( &delayed_queue::poll_one, &asio_queue::poll_one);
}

void bique::stop()
{
  return this->invoke_( &delayed_queue::stop, &asio_queue::stop);
}

bool bique::post( function_t f )
{
  return this->invoke_( &delayed_queue::post, &asio_queue::post, std::move(f) );
}

bool bique::post_at(time_point_t tp, function_t&& f)
{
  return this->invoke_( &delayed_queue::post_at, &asio_queue::post_at, tp, std::move(f));
}

bool bique::delayed_post(duration_t duration, function_t&& f)
{
  return this->invoke_( &delayed_queue::delayed_post, &asio_queue::delayed_post, duration, std::move(f));
}

std::size_t bique::size() const
{
  return this->invoke_( &delayed_queue::size, &asio_queue::size);
}

std::size_t bique::dropped() const
{
  return this->invoke_( &delayed_queue::dropped, &asio_queue::dropped);
}
//private:
  
template<typename R, typename... Args>
R bique::invoke_( 
  R(delayed_queue::* method1)(Args...),
  R(asio_queue::* method2)(Args...), 
  Args... args)
{
  return _dflag 
         ? (_delayed.get()->*method1)( std::move(args)...)
         : _mt_flag
             ? (_asio.get()->*method2)( std::move(args)...)
             : (_asio_st.get()->*method2)( std::move(args)...);
}

template<typename R, typename... Args>
R bique::invoke_( 
  R(delayed_queue::* method1)(Args...) const,
  R(asio_queue::* method2)(Args...) const, 
  Args... args) const
{
  return _dflag 
         ? (_delayed.get()->*method1)( std::move(args)...)
         : _mt_flag
           ? (_asio.get()->*method2)( std::move(args)...)
           : (_asio_st.get()->*method2)( std::move(args)...);
}

}
