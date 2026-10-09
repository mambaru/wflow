#pragma once 

#include <chrono>
#include <atomic>
#include <deque>
#include <queue>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <vector>
#include <cstddef>


namespace wflow {

/**
 * @brief Нативная (без Boost.Asio) шардированная очередь заданий с поддержкой отложенного запуска.
 *
 * ## Зачем нужна
 *
 * Альтернатива @ref asio_queue / `boost::asio::io_context` для сценариев, где очередь заданий —
 * основной путь исполнения, а не приложение к сокетам. Реализует тот же контракт
 * (`post` / `safe_post`, timed post, `run` / `poll_one`, maxsize, drop), но на своих структурах:
 * ready-очередь + priority-heap таймеров, разнесённые по шардам.
 *
 * На легковесных заданиях (пустой/`короткий` handler) обычно даёт заметно большую пропускную
 * способность, чем asio, особенно при нескольких publisher-потоках: издатели почти не
 * конкурируют за один mutex (affinity шарда по `std::thread::id`), а global CV для wait
 * трогается только когда consumer действительно спит.
 *
 * Включается через `workflow_options::use_native = true` (или напрямую, без `workflow`).
 *
 * ## Когда использовать
 *
 * - CPU-/очередные пайплайны без сети: воркеры, фоновые джобы, in-process шина событий.
 * - Высокий RPS на коротких handlers, 1..N publisher-потоков.
 * - Нужны delayed/safe post и лимит `maxsize`, но asio в процессе не обязателен.
 * - Отдельный `workflow` под таймеры/контроль рядом с «тяжёлой» asio-очередью запросов.
 *
 * ## Когда лучше asio_queue
 *
 * - Уже крутится `io_context` под сокеты/таймеры asio — одна модель исполнения проще.
 * - Долгие handlers и мало post'ов: выигрыш native почти не виден, важнее единообразие.
 * - Нужна интеграция с asio-composed операциями в том же reactor'е.
 *
 * ## Потоки и шарды
 *
 * - `shards == 0` — auto: `hardware_concurrency`, ограничение 1..16.
 * - Publisher пишет в шард `hash(thread_id) % shards`.
 * - Consumer обходит шарды round-robin; ожидание — на общем condition_variable.
 * - `maxsize` действует на суммарное число unsafe-заданий (как в asio_queue); safe не занимает слот.
 *
 * ## Замечания
 *
 * - Требует `std::shared_ptr` владения (как и остальной стек wflow).
 * - `reset()` безопасен при работающих consumer'ах; отбрасывает queued handlers без вызова drop.
 * - Имя исторически было `delayed_queue`; переименовано, т.к. delayed — лишь одна из функций.
 */
class native_queue
  : public std::enable_shared_from_this<native_queue >
{
  typedef native_queue self;
public:

  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef std::condition_variable                             condition_variable_t;
  typedef std::mutex                                          mutex_t;

  native_queue( native_queue const & ) = delete;
  void operator=( native_queue const & ) = delete;

  /**
   * @param maxsize лимит unsafe-заданий (0 — без лимита)
   * @param shards число шардов ready/timed очередей; 0 — auto (hardware_concurrency, 1..16)
   */
  explicit native_queue(size_t maxsize, size_t shards = 0);

  virtual ~native_queue ();
  
  void set_maxsize(size_t maxsize);

  void reset();

  /// Снять все задания с очередей и вызвать handlers (для hard-reconfigure / owner::wrap alt).
  void discard_queued();

  std::size_t run();
  
  std::size_t run_one();

  std::size_t run_one_for_ms(time_t ms);

  std::size_t run_for_ms(time_t ms);

  std::size_t poll_one();

  void stop();

  bool stopped() const;

  void safe_post( function_t f );
  
  void safe_post_at(time_point_t time_point, function_t f);

  void safe_delayed_post(duration_t duration, function_t f);
  
  bool post( function_t f, function_t drop );
  
  bool post_at(time_point_t time_point, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);
  
  std::size_t safe_size() const;
  std::size_t unsafe_size() const;
  std::size_t full_size() const;
  std::size_t dropped() const;

  std::size_t shard_count() const;

  static bool work() { return false;}
private:
  struct queued_handler
  {
    function_t func;
    bool safe;
  };

  typedef std::pair<time_point_t, queued_handler>             event_t;

  struct queue_cmp
  {
    inline bool operator()( const event_t& e1, const event_t& e2 ) const
    {
      return e1.first > e2.first;
    }
  };

  typedef std::queue<queued_handler>                          queue_t;
  typedef std::priority_queue<event_t, std::deque<event_t>, queue_cmp>  timed_queue_t;

  struct alignas(64) shard
  {
    mutable mutex_t mutex;
    queue_t que;
    timed_queue_t timed_que;
  };

  static size_t default_shard_count_(size_t shards);

  size_t pick_shard_() const;

  /// Атомарно занять слот unsafe-очереди (между шардами).
  bool acquire_();

  void push_at_(shard& s, time_point_t time_point, queued_handler handler);

  bool migrate_ready_(shard& s, time_point_t now);
  
  std::size_t poll_one_();

  /// one — выйти после первого handler; ms>0 — не дольше slice (для status_ms / shrink).
  std::size_t loop_(bool one, time_t ms);

  /// slice_until=nullptr — ждать работу бесконечно (run/run_one).
  void run_wait_(size_t epoch, const std::chrono::steady_clock::time_point* slice_until);

  /// ближайший timed среди шардов; false если timed-заданий нет
  bool next_deadline_(time_point_t* tp) const;

  void notify_wait_();

private:

  std::vector<shard>       _shards;
  mutable std::atomic<size_t> _poll_shard;
  std::atomic<size_t>      _epoch;
  std::atomic<size_t>      _waiters;

  mutex_t                  _wait_mutex;
  condition_variable_t     _wait_cv;

  std::atomic<bool>        _loop_exit;

  std::atomic<size_t> _counter;
  std::atomic<size_t> _safe_counter;

  std::atomic<size_t> _maxsize;
  std::atomic<size_t> _drop_count;
}; // native_queue

/// @deprecated Используйте native_queue
using delayed_queue = native_queue;

}
