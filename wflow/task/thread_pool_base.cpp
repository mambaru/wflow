#include "thread_pool_base.hpp"

#include <wflow/queue/native_queue.hpp>
#include <wflow/queue/bique.hpp>
#include <wflow/queue/asio_queue.hpp>
#include <wflow/logger.hpp>

#include <sys/syscall.h>
#include <sys/types.h>
#include <chrono>
#include <iostream>


namespace wflow {

namespace{
template<typename T>
inline void nothing(const T& ){}
}

thread_pool_base::thread_pool_base()
  : _started(false)
  , _status_ms(0)
{
}

void thread_pool_base::set_startup( startup_handler handler )
{
  _startup = handler;
}

void thread_pool_base::set_status( status_handler handler, time_t status_ms )
{
  _status = handler;
  _status_ms = status_ms;
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

bool thread_pool_base::reconfigure(std::shared_ptr<native_queue> s, size_t threads)
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

void thread_pool_base::start(std::shared_ptr<native_queue> s, size_t threads)
{
  this->start_(s, threads);
}

// только после _service->stop();
void thread_pool_base::stop()
{
  std::vector<std::thread> threads;
  {
    std::lock_guard< std::mutex > lk(_mutex);
    _flags.clear();
    _work=nullptr;
    threads.swap(_threads);
    _started = false;
  }
  // join вне mutex: иначе stop/wait из handler'а пула — self-deadlock
  for (auto& t : threads)
    t.join();
}

void thread_pool_base::shutdown()
{
  std::lock_guard< std::mutex > lk(_mutex);
  _work=nullptr;
}

void thread_pool_base::wait()
{
  std::vector<std::thread> threads;
  {
    std::lock_guard< std::mutex > lk(_mutex);
    if ( _work!=nullptr )
      return;
    threads.swap(_threads);
    _started = false;
  }
  for (auto& t : threads)
    t.join();
}


template<typename S>
bool thread_pool_base::reconfigure_(std::shared_ptr<S> s, size_t threads)
{
  bool need_reset = false;
  size_t grow = 0;
  size_t poke = 0;
  std::vector<std::thread> join_threads;

  {
    std::lock_guard< std::mutex > lk(_mutex);

    if ( !_started )
      return true;

    if ( threads == _threads.size() )
      return true;

    if ( threads > _threads.size() )
    {
      // При серии реконфигураций N->0->N потоков, сбрасываем io_context для нового запуска
      if ( _threads.empty() )
        need_reset = true;
      grow = threads - _threads.size();
    }
    else if ( threads == 0 )
    {
      // N→0: дожидаемся потоков — иначе run() ещё крутит io, а вызывающий уже постит вручную
      _flags.clear();
      _work = nullptr;
      join_threads.swap(_threads);
    }
    else
    {
      // N→M (M>0): гасим лишние флаги и join вне mutex — как N→0, без detach
      const size_t oldsize = _threads.size();
      _flags.resize(threads);
      for ( size_t i = threads; i < oldsize; ++i )
        join_threads.push_back(std::move(_threads[i]));
      _threads.resize(threads);
      poke = oldsize * 2;
    }
  }

  // Сначала будим уходящие потоки (work_guard ещё жив), потом join
  for (; poke != 0; --poke )
  {
    s->safe_post([]() noexcept{});
    std::this_thread::sleep_for( std::chrono::milliseconds(1) );
  }

  for (auto& t : join_threads)
    t.join();

  // reset / grow вне mutex — иначе finish_handler или stop дедлочат
  if ( need_reset )
    s->reset();

  if ( grow != 0 )
  {
    std::lock_guard< std::mutex > lk(_mutex);
    if ( !_started )
      return false;
    this->run_more_(s, grow);
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


template<typename S>
void thread_pool_base::run_more_(std::shared_ptr<S> s, size_t threads)
{
  size_t prev_size = _threads.size();
  _threads.reserve( prev_size + threads);
  for (size_t i = 0 ; i < threads; ++i)
  {
    thread_flag pflag = std::make_shared<bool>(true);
    _flags.push_back(pflag);
    if ( _work==nullptr )
    {
      auto w = s->work();
      _work=[w](){ nothing(w);};
    }
    _threads.push_back( this->create_thread_(s, pflag) );
  }
}

template<typename S>
std::thread thread_pool_base::create_thread_( std::shared_ptr<S> s, std::weak_ptr<bool> wflag )
{
  std::weak_ptr<self> wthis = this->shared_from_this();
  return
    std::thread([wthis, s, wflag]()
    {
      std::thread::id thread_id = std::this_thread::get_id();
      try
      {
        thread_pool_base::startup_handler startup;
        thread_pool_base::status_handler status;
        thread_pool_base::finish_handler finish;
        thread_pool_base::statistics_handler statistics;

        if ( auto pthis = wthis.lock() )
        {
          startup = pthis->_startup;
          status = pthis->_status;
          finish = pthis->_finish;
          statistics = pthis->_statistics;
        }

        if ( startup != nullptr )
          startup(thread_id);

        std::chrono::steady_clock::time_point beg = std::chrono::steady_clock::now();
        time_t status_time = time(nullptr);
        for (;;)
        {
          if ( statistics != nullptr )
            beg = std::chrono::steady_clock::now();

          size_t handlers = 0;
          time_t status_ms = 0;
          if ( auto pthis = wthis.lock() )
            status_ms = pthis->_status_ms;

          // status_ms==0 отключает status_handler, но slice всё равно нужен:
          // иначе run() не возвращается и soft shrink / N→0 зависают на join.
          const time_t slice_ms = status_ms != 0 ? status_ms : 1000;
          if ( statistics != nullptr )
            handlers = s->run_one_for_ms( slice_ms );
          else
            handlers = s->run_for_ms( slice_ms );

          if ( status != nullptr && status_ms != 0 )
          {
            if ( time(nullptr) - status_time > 0)
            {
              status(thread_id);
              status_time = time(nullptr);
            }
          }

          if ( s->stopped() )
            break;
          if ( wflag.lock() == nullptr)
            break;
          if ( statistics != nullptr && handlers > 0 )
          {
            auto now = std::chrono::steady_clock::now();
            auto span = now - beg ;
            statistics( thread_id, handlers, span );
          }
        }
        if ( finish != nullptr )
          finish(thread_id);
      }
      catch(const std::exception& e)
      {
        WFLOW_LOG_FATAL("Exception in workflow thread: " << e.what() )
      }
      catch(...)
      {
        WFLOW_LOG_FATAL("Unhandled exception in workflow thread." )
      }
    });
}


}
