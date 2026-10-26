#include "native_queue.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

namespace wflow {

size_t native_queue::default_shard_count_(size_t shards)
{
  if (shards != 0)
    return shards;
  unsigned hc = std::thread::hardware_concurrency();
  if (hc == 0)
    hc = 4;
  return std::max<size_t>(1, std::min<size_t>(16, hc));
}

native_queue::native_queue(size_t maxsize, size_t shards)
  : _shards(default_shard_count_(shards))
  , _poll_shard(0)
  , _epoch(0)
  , _waiters(0)
  , _loop_exit(false)
  , _counter(0)
  , _safe_counter(0)
  , _maxsize(maxsize)
  , _drop_count(0)
{
}

native_queue::~native_queue ()
{
  this->stop();
}

size_t native_queue::shard_count() const
{
  return _shards.size();
}
  
void native_queue::set_maxsize(size_t maxsize)
{
  _maxsize = maxsize;
}

void native_queue::reset()
{
  for (auto& s : _shards)
  {
    std::lock_guard<mutex_t> lck(s.mutex);
    queue_t empty_que;
    timed_queue_t empty_timed;
    s.que.swap(empty_que);
    s.timed_que.swap(empty_timed);
  }
  _counter = 0;
  _safe_counter = 0;
  _drop_count = 0;
  _loop_exit = false;
  this->notify_wait_();
}

void native_queue::discard_queued()
{
  std::vector<function_t> handlers;
  for (auto& s : _shards)
  {
    std::lock_guard<mutex_t> lck(s.mutex);
    while ( !s.que.empty() )
    {
      handlers.push_back( std::move(s.que.front().func) );
      s.que.pop();
    }
    while ( !s.timed_que.empty() )
    {
      handlers.push_back( std::move(s.timed_que.top().second.func) );
      s.timed_que.pop();
    }
  }
  _counter = 0;
  _safe_counter = 0;
  this->notify_wait_();

  for (const auto& f : handlers)
  {
    if ( f )
      f();
  }
}

std::size_t native_queue::run()
{
  if ( _loop_exit ) 
    return 0;
  return this->loop_(false, 0);
}
  
std::size_t native_queue::run_one()
{
  if ( _loop_exit ) 
    return 0;
  return this->loop_(true, 0);
}

std::size_t native_queue::poll_one()
{
  if ( _loop_exit ) 
    return 0;
  return this->poll_one_();
}


std::size_t native_queue::run_one_for_ms(time_t ms)
{
  if ( _loop_exit )
    return 0;
  if ( ms <= 0 )
    return this->poll_one();
  return this->loop_(true, ms);
}

std::size_t native_queue::run_for_ms(time_t ms)
{
  if ( _loop_exit )
    return 0;
  if ( ms <= 0 )
    return 0;
  return this->loop_(false, ms);
}

void native_queue::stop()
{
  _loop_exit = true;
  this->notify_wait_();
}

bool native_queue::stopped() const
{
  return _loop_exit;
}

size_t native_queue::pick_shard_() const
{
  // affinity по потоку: один publisher не размазывает по всем шардам
  const auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
  return h % _shards.size();
}

void native_queue::notify_wait_()
{
  // на горячем пути без waiters не трогаем глобальный mutex
  if ( _waiters.load(std::memory_order_acquire) == 0 )
  {
    _epoch.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  // порядок локов: никогда не держим shard.mutex вместе с _wait_mutex
  std::lock_guard<mutex_t> lk(_wait_mutex);
  _epoch.fetch_add(1, std::memory_order_relaxed);
  _wait_cv.notify_all();
}

void native_queue::safe_post( function_t f)
{
  {
    shard& s = _shards[this->pick_shard_()];
    std::lock_guard<mutex_t> lock( s.mutex );
    ++_safe_counter;
    s.que.push( queued_handler{ std::move(f), true } );
  }
  this->notify_wait_();
}

bool native_queue::post( function_t f, function_t drop )
{
  if ( !this->acquire_() )
  {
    ++_drop_count;
    if (drop)
      drop();
    return false;
  }

  {
    shard& s = _shards[this->pick_shard_()];
    std::lock_guard<mutex_t> lock( s.mutex );
    s.que.push( queued_handler{ std::move(f), false } );
  }
  this->notify_wait_();
  return true;
}

void native_queue::safe_post_at(time_point_t time_point, function_t f)
{
  {
    shard& s = _shards[this->pick_shard_()];
    std::lock_guard<mutex_t> lock( s.mutex );
    ++_safe_counter;
    this->push_at_( s, time_point, queued_handler{ std::move(f), true } );
  }
  this->notify_wait_();
}

bool native_queue::post_at(time_point_t time_point, function_t f, function_t drop)
{
  if ( !this->acquire_() )
  {
    ++_drop_count;
    if (drop)
      drop();
    return false;
  }

  {
    shard& s = _shards[this->pick_shard_()];
    std::lock_guard<mutex_t> lock( s.mutex );
    this->push_at_( s, time_point, queued_handler{ std::move(f), false } );
  }
  this->notify_wait_();
  return true;
}

void native_queue::safe_delayed_post(duration_t duration, function_t f)
{  
  if ( 0 == duration.count() )
    this->safe_post( std::move(f) );
  else
    this->safe_post_at( std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(duration), std::move(f));
}

bool native_queue::delayed_post(duration_t duration, function_t f, function_t drop)
{  
  if ( 0 == duration.count() )
    return this->post( std::move(f), std::move(drop) );
  else
    return this->post_at( std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::microseconds>(duration), std::move(f), std::move(drop) );
}

std::size_t native_queue::unsafe_size() const
{
  return _counter;
}

std::size_t native_queue::safe_size() const
{
  return _safe_counter;
}

std::size_t native_queue::full_size() const
{
  return _safe_counter + _counter;
}

std::size_t native_queue::dropped() const
{
  return _drop_count;
}

bool native_queue::acquire_()
{
  const size_t maxsize = _maxsize.load(std::memory_order_relaxed);
  if ( maxsize == 0 )
  {
    ++_counter;
    return true;
  }

  size_t cur = _counter.load(std::memory_order_relaxed);
  while ( cur < maxsize )
  {
    if ( _counter.compare_exchange_weak(cur, cur + 1,
           std::memory_order_acq_rel, std::memory_order_relaxed) )
      return true;
  }
  return false;
}

void native_queue::push_at_(shard& s, time_point_t time_point, queued_handler handler)
{
  s.timed_que.emplace( time_point, std::move( handler ) );
}

bool native_queue::migrate_ready_(shard& s, time_point_t now)
{
  bool moved = false;
  while ( !s.timed_que.empty() && s.timed_que.top().first <= now )
  {
    s.que.push( std::move( s.timed_que.top().second ) );
    s.timed_que.pop();
    moved = true;
  }
  return moved;
}
  
std::size_t native_queue::poll_one_()
{
  const size_t n = _shards.size();
  const size_t start = _poll_shard.fetch_add(1, std::memory_order_relaxed) % n;
  const time_point_t now = std::chrono::system_clock::now();

  for (size_t i = 0; i < n; ++i)
  {
    shard& s = _shards[(start + i) % n];
    std::unique_lock<mutex_t> lock(s.mutex);
    this->migrate_ready_(s, now);
    if ( s.que.empty() )
      continue;

    queued_handler job = std::move( s.que.front() );
    s.que.pop();
    if ( job.safe )
      --_safe_counter;
    else
      --_counter;
    lock.unlock();

    if ( job.func )
      job.func();
    return 1;
  }
  return 0;
}

std::size_t native_queue::loop_(bool one, time_t ms)
{
  using steady = std::chrono::steady_clock;
  const bool limited = ms > 0;
  const auto slice_end = limited
    ? steady::now() + std::chrono::milliseconds(ms)
    : steady::time_point{};

  std::size_t result = 0;
  while ( !_loop_exit )
  {
    if ( limited && steady::now() >= slice_end )
      break;

    // epoch до poll: post между пустым poll и wait бампит epoch —
    // run_wait_ увидит mismatch и не уснёт с работой в ready-очереди
    const size_t epoch = _epoch.load(std::memory_order_relaxed);
    if ( !this->poll_one_() )
    {
      this->run_wait_(epoch, limited ? &slice_end : nullptr);
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

bool native_queue::next_deadline_(time_point_t* tp) const
{
  bool found = false;
  time_point_t best;
  for (const auto& s : _shards)
  {
    std::lock_guard<mutex_t> lock(s.mutex);
    if ( s.timed_que.empty() )
      continue;
    const time_point_t cur = s.timed_que.top().first;
    if ( !found || cur < best )
    {
      best = cur;
      found = true;
    }
  }
  if (found && tp)
    *tp = best;
  return found;
}

void native_queue::run_wait_(size_t epoch, const std::chrono::steady_clock::time_point* slice_until)
{
  using steady = std::chrono::steady_clock;
  // не держим _wait_mutex при обходе шардов (порядок локов)
  time_point_t job_deadline;
  const bool has_job = this->next_deadline_(&job_deadline);

  std::unique_lock<mutex_t> lk(_wait_mutex);
  _waiters.fetch_add(1, std::memory_order_acq_rel);
  if ( _loop_exit || _epoch.load(std::memory_order_relaxed) != epoch )
  {
    _waiters.fetch_sub(1, std::memory_order_acq_rel);
    return;
  }

  const auto pred = [this, epoch]() {
    return _loop_exit || _epoch.load(std::memory_order_relaxed) != epoch;
  };

  if ( slice_until != nullptr )
  {
    const auto now = steady::now();
    if ( now >= *slice_until )
    {
      _waiters.fetch_sub(1, std::memory_order_acq_rel);
      return;
    }
    const auto slice_left = *slice_until - now;

    if ( has_job )
    {
      const auto job_left = job_deadline - std::chrono::system_clock::now();
      if ( job_left <= decltype(job_left)::zero() )
      {
        _waiters.fetch_sub(1, std::memory_order_acq_rel);
        return;
      }
      const auto wait_span = std::min(
        std::chrono::duration_cast<steady::duration>(job_left),
        slice_left);
      _wait_cv.wait_for(lk, wait_span, pred);
    }
    else
    {
      _wait_cv.wait_for(lk, slice_left, pred);
    }
  }
  else if ( has_job )
  {
    _wait_cv.wait_until(lk, job_deadline, pred);
  }
  else
  {
    _wait_cv.wait(lk, pred);
  }
  _waiters.fetch_sub(1, std::memory_order_acq_rel);
}

}
