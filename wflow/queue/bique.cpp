#include "bique.hpp"
#include "asio_queue.hpp"
#include "native_queue.hpp"


namespace wflow{

bique::~bique()
{
  this->stop();
}

bique::bique( size_t maxsize, bool use_native)
  : _use_native(use_native)
  , _mt_flag(true)
  , _io( std::make_shared<io_context_type>() )
  , _native( std::make_shared<native_queue>(maxsize) )
  , _asio( std::make_shared<asio_queue>( *_io, maxsize) )
  , _asio_st(nullptr)
{
}

bique::bique( io_context_type& io, size_t maxsize, bool use_native, bool mt )
  : _use_native(use_native)
  , _mt_flag(mt)
  , _io( std::make_shared<io_context_type>() )
  , _native( std::make_shared<native_queue>(maxsize) )
  , _asio( std::make_shared<asio_queue>( *_io, maxsize) )
  , _asio_st(std::make_shared<asio_queue>( io, maxsize) )
{
}

bique::asio_ptr bique::asio_queue_() const
{
  // threads==0: внешний io, если ctor был с io; иначе внутренний _asio (без null deref).
  if ( !_mt_flag.load(std::memory_order_relaxed) && _asio_st )
    return _asio_st;
  return _asio;
}

bique::io_context_type& bique::get_io_context()
{
  // Как раньше: native / ST предпочитают _asio_st; если его нет — внутренний _asio.
  if ( (_use_native || !_mt_flag) && _asio_st )
    return _asio_st->get_io_context();
  return _asio->get_io_context();
}

bique::work_type bique::work() const
{
  if ( _mt_flag || !_asio_st )
    return _asio->work();
  return _asio_st->work();
}

void bique::reconfigure(size_t maxsize, bool use_native, bool mt )
{
  const bool was_mt = _mt_flag.load(std::memory_order_relaxed);
  _use_native = use_native;
  _mt_flag = mt;
  _native->set_maxsize(maxsize);
  _asio->set_maxsize(maxsize);
  if( _asio_st )
    _asio_st->set_maxsize(maxsize);

  // Пул остановил io_context::run(); без restart повторный run() ничего не сделает.
  // Внешний io (есть _asio_st) крутит вызывающий — его не трогаем.
  if ( was_mt && !mt && !_asio_st )
    _asio->reset();
}

void bique::reset()
{
  _native->reset();
  _asio->reset();
}

void bique::discard_queued()
{
  _native->discard_queued();
  // asio_queue нельзя «снять» handlers из io_context; hard-path уже owner.reset —
  // оставшиеся post при следующем run уйдут в wrap→alt. stop+reset только
  // внутреннего io (внешний _asio_st не трогаем): разблокировать stopped run.
  _asio->stop();
  _asio->reset();
}

std::size_t bique::run()
{
  return this->invoke_( &native_queue::run, &asio_queue::run);
}

std::size_t bique::run_one()
{
  return this->invoke_( &native_queue::run_one, &asio_queue::run_one);
}

std::size_t bique::poll_one()
{
  return this->invoke_( &native_queue::poll_one, &asio_queue::poll_one);
}

std::size_t bique::run_one_for_ms(time_t ms)
{
  return this->invoke_( &native_queue::run_one_for_ms, &asio_queue::run_one_for_ms, std::move(ms));
}

std::size_t bique::run_for_ms(time_t ms)
{
  return this->invoke_( &native_queue::run_for_ms, &asio_queue::run_for_ms, std::move(ms));
}

bool bique::stopped() const
{
  return this->invoke_( &native_queue::stopped, &asio_queue::stopped);
}

void bique::stop()
{
  _native->stop();
  _asio->stop();
}

void bique::safe_post( function_t f )
{
  return this->invoke_( &native_queue::safe_post, &asio_queue::safe_post, std::move(f));
}

void bique::safe_post_at(time_point_t tp, function_t f)
{
  return this->invoke_( &native_queue::safe_post_at, &asio_queue::safe_post_at, std::move(tp), std::move(f) );
}

void bique::safe_delayed_post(duration_t duration, function_t f)
{
  return this->invoke_( &native_queue::safe_delayed_post, &asio_queue::safe_delayed_post, std::move(duration), std::move(f) );
}

bool bique::post( function_t f, function_t drop )
{
  return this->invoke_( &native_queue::post, &asio_queue::post, std::move(f), std::move(drop) );
}

bool bique::post_at(time_point_t tp, function_t f, function_t drop)
{
  return this->invoke_( &native_queue::post_at, &asio_queue::post_at, std::move(tp), std::move(f), std::move(drop));
}

bool bique::delayed_post(duration_t duration, function_t f, function_t drop)
{
  return this->invoke_( &native_queue::delayed_post, &asio_queue::delayed_post, std::move(duration), std::move(f), std::move(drop));
}

std::size_t bique::full_size() const
{
  return this->invoke_( &native_queue::full_size, &asio_queue::full_size);
}

std::size_t bique::safe_size() const
{
  return this->invoke_( &native_queue::safe_size, &asio_queue::safe_size);
}

std::size_t bique::unsafe_size() const
{
  return this->invoke_( &native_queue::unsafe_size, &asio_queue::unsafe_size);
}

std::size_t bique::dropped() const
{
  return this->invoke_( &native_queue::dropped, &asio_queue::dropped);
}

template<typename R, typename... Args>
R bique::invoke_(
  R(native_queue::* method1)(Args...),
  R(asio_queue::* method2)(Args...),
  Args&&... args)
{
  if ( _use_native )
    return (_native.get()->*method1)( std::forward<Args>(args)...);
  return (this->asio_queue_().get()->*method2)( std::forward<Args>(args)...);
}

template<typename R, typename... Args>
R bique::invoke_(
  R(native_queue::* method1)(Args...) const,
  R(asio_queue::* method2)(Args...) const,
  Args&&... args) const
{
  if ( _use_native )
    return (_native.get()->*method1)( std::forward<Args>(args)...);
  return (this->asio_queue_().get()->*method2)( std::forward<Args>(args)...);
}

}
