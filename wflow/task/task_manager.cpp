#include <wflow/task/task_manager.hpp>
#include <wflow/task/thread_pool.hpp>
#include <wflow/timer/timer_manager.hpp>
#include <wflow/queue/bique.hpp>
#include <wflow/logger.hpp>
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
  : _id(opt.id)
  , _threads(opt.threads)
  , _use_native(opt.use_native)
  , _queue( std::make_shared<queue_type>(opt.maxsize, opt.use_native) )
  , _timer_manager( std::make_shared<timer_manager_t >(_queue) )
  , _pool( std::make_shared<pool_type>(_queue) )
  , _rate_limit(opt.rate_limit)
  , _quiet_mode(opt.quiet_mode)
  , _overflow_reset(opt.overflow_reset)
  , _reset_count(std::make_shared< std::atomic<size_t> >(0) )
{
}

task_manager::task_manager( io_context_type& io, const workflow_options& opt  )
  : _id(opt.id)
  , _threads(opt.threads)
  , _use_native(opt.use_native)
  , _queue( std::make_shared<queue_type>(io, opt.maxsize, opt.use_native, opt.threads!=0) )
  , _timer_manager( std::make_shared<timer_manager_t >(_queue) )
  , _pool( std::make_shared<pool_type>(_queue) )
  , _rate_limit(opt.rate_limit)
  , _quiet_mode(opt.quiet_mode)
  , _overflow_reset(opt.overflow_reset)
  , _reset_count(std::make_shared< std::atomic<size_t> >(0) )
{
}

bool task_manager::reconfigure(const workflow_options& opt  )
{
  const size_t old_threads = _threads.load(std::memory_order_relaxed);
  const bool old_native = _use_native.load(std::memory_order_relaxed);
  const bool hard =
    ( old_native != opt.use_native ) ||
    ( (old_threads == 0) != (opt.threads == 0) );

  if ( hard )
  {
    // Жёсткий переход: пул стопим, старые тики таймеров «протухают»,
    // native-задания вызываем (wrap → alt), затем таймеры с теми же id встают снова.
    if ( old_threads > 0 )
      _pool->reconfigure(0);
    _timer_manager->rebind_flags();
    _queue->discard_queued();
  }
  else if ( old_threads != opt.threads )
  {
    _pool->reconfigure(opt.threads);
  }

  _queue->reconfigure(opt.maxsize, opt.use_native, opt.threads != 0);
  _use_native = opt.use_native;
  _threads = opt.threads;

  if ( hard && opt.threads > 0 )
    _pool->reconfigure(opt.threads);

  if ( hard )
    _timer_manager->rearm_all();

  _rate_limit = opt.rate_limit;
  this->reset_rate_limit_state_();

  _quiet_mode = opt.quiet_mode;
  _overflow_reset = opt.overflow_reset;
  _overflow_time = 0;
  _wait_reset = false;

  _id = opt.id;
  return true;
}

task_manager::io_context_type& task_manager::get_io_context()
{
  return _queue->get_io_context();
}


void task_manager::rate_limit(size_t rps)
{
  _rate_limit = rps;
  this->reset_rate_limit_state_();
}

void task_manager::reset_rate_limit_state_()
{
  std::lock_guard<std::mutex> lk(_rate_mutex);
  _rate_window_start = {};
  _rate_count = 0;
}

void task_manager::set_startup( startup_handler handler )
{
  if ( _pool!=nullptr)
    _pool->set_startup(handler);
}

void task_manager::set_status( status_handler handler, time_t status_ms )
{
  if ( _pool!=nullptr)
    _pool->set_status(handler, status_ms);
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
  if ( _pool == nullptr )
    return;

  // После stop() очередь остаётся stopped (io.stop / _loop_exit) —
  // оживляем только в этом случае, чтобы не сбрасывать работающую очередь.
  if ( _queue->stopped() )
    _queue->reset();

  _pool->start(_threads);
}

void task_manager::stop()
{
  // По доке stop сбрасывает очереди/таймеры и останавливает потоки
  _timer_manager->reset();
  _queue->stop();
  if ( _pool!=nullptr)
    _pool->stop();
}

void task_manager::reset()
{
  _timer_manager->reset();
  _queue->reset();
}

void task_manager::reset_timers()
{
  _timer_manager->reset();
}

void task_manager::reset_queues()
{
  _queue->reset();
}

void task_manager::shutdown()
{
  _timer_manager->reset();
  if ( _pool!=nullptr)
    _pool->shutdown();
}

void task_manager::wait()
{
  if ( _pool!=nullptr)
    _pool->wait();
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
  bool succeeded = this->post_(f, drop);
  if ( !succeeded )
  {
    if ( _overflow_reset && !_wait_reset)
    {
      // Инвалидируем поколение (shared_ptr не трогаем — только atomic)
      const size_t n = _reset_count->fetch_add(1, std::memory_order_relaxed) + 1;
      // Пока очередь забита старыми обёртками — как обычный overflow без повторного сброса
      _wait_reset = true;
      if ( !_quiet_mode )
      {
        WFLOW_LOG_ERROR("Workflow '" << _id << "' queue reset N" << n << " due to overflow. Total dropped: " << this->dropped() )
      }
    }
    else if ( !_quiet_mode )
    {
      time_t now = time(nullptr);
      if ( _overflow_time == 0 || _overflow_time < now  )
      {
        _overflow_time = now + 1;
        WFLOW_LOG_ERROR("Workflow '" << this->_id << "' task dropped. Total dropped: " << this->dropped() )
      }
    }
  }
  else if ( _overflow_reset )
  {
    // Первый же успешный post_, что в очереди появилось место
    // а значит началась отработка сброшенных заданий (запуск альтернативных обработчиков )
    _wait_reset = false;
  }

  return succeeded;
}

bool task_manager::post_at(time_point_t tp, function_t f, function_t drop)
{
  this->safe_post_at(tp, std::bind(&task_manager::post, this, f, drop) );
  return true;
}

bool task_manager::delayed_post(duration_t duration, function_t f, function_t drop)
{
  this->safe_delayed_post(duration, std::bind(&task_manager::post, this, f, drop) );
  return true;
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

std::size_t task_manager::reset_count() const
{
  return _reset_count->load(std::memory_order_relaxed);
}


std::shared_ptr<task_manager::timer_manager_t> task_manager::get_timer_manager() const
{
  return _timer_manager;
}

bool task_manager::post_(function_t f, function_t drop)
{
  using namespace std::chrono;
  if ( _overflow_reset )
  {
    // Копия shared_ptr без последующего assign в других потоках — data race нет (C++14).
    std::weak_ptr< std::atomic<size_t> > w = _reset_count;
    const size_t gen = _reset_count->load(std::memory_order_relaxed);
    f = [w, gen, f, drop]()
    {
      if (auto p = w.lock())
      {
        if (p->load(std::memory_order_relaxed) == gen) f();
        else if (drop) drop();
      }
      else if (drop) drop();
    };
  }

  const size_t limit = _rate_limit.load(std::memory_order_relaxed);
  if ( limit == 0 )
    return _queue->post(std::move(f), std::move(drop));

  duration_t delay = duration_t::zero();
  {
    std::lock_guard<std::mutex> lk(_rate_mutex);
    const auto now = steady_clock::now();
    if ( _rate_count == 0 || now - _rate_window_start >= seconds(1) )
    {
      _rate_window_start = now;
      _rate_count = 0;
    }

    ++_rate_count;
    if ( _rate_count > limit )
    {
      const auto due = _rate_window_start
        + nanoseconds( (_rate_count * 1000000000ull) / limit );
      if ( due > now )
        delay = duration_cast<duration_t>(due - now);
    }
  }

  if ( delay == duration_t::zero() )
    return _queue->post(std::move(f), std::move(drop));

  // Слот уже занят в окне — в очередь напрямую, без повторного rate_limit.
  auto queue = _queue;
  this->safe_delayed_post(delay, [queue, f, drop]() mutable
  {
    queue->post(std::move(f), std::move(drop));
  });
  return true;
}

}
