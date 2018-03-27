#pragma once 

#include <chrono>
#include <atomic>
#include <queue>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>


namespace wflow {

class delayed_queue
  : public std::enable_shared_from_this<delayed_queue >
{
  struct queue_cmp;
  typedef delayed_queue self;
public:

  typedef std::function<void()>                               function_t;
  typedef std::chrono::time_point<std::chrono::system_clock>  time_point_t;
  typedef time_point_t::duration                              duration_t;
  typedef std::pair<time_point_t, function_t>                 event_t;
  typedef std::condition_variable                             condition_variable_t;
  typedef std::mutex                                          mutex_t;
  typedef std::queue<function_t>                              queue_t;
  typedef std::priority_queue<event_t, std::deque<event_t>, queue_cmp>  delayed_queue_t;

  delayed_queue( delayed_queue const & ) = delete;
  void operator=( delayed_queue const & ) = delete;

  explicit delayed_queue(size_t maxsize);

  virtual ~delayed_queue ();
  
  void set_maxsize(size_t maxsize);

  void reset();

  std::size_t run();
  
  std::size_t run_one();

  std::size_t poll_one();

  void stop();

  void safe_post( function_t f );
  
  void safe_post_at(time_point_t time_point, function_t f);

  void safe_delayed_post(duration_t duration, function_t f);
  
  bool post( function_t f);
  
  bool post_at(time_point_t time_point, function_t f);

  bool delayed_post(duration_t duration, function_t f);
  
  std::size_t safe_size() const;
  std::size_t unsafe_size() const;
  std::size_t full_size() const;
  std::size_t dropped() const;

  static bool work() { return false;}
private:
  bool check_();

  std::size_t size_() const;

  void push_at_(time_point_t time_point, function_t f);
  
  std::size_t poll_one_( std::unique_lock<mutex_t>& lck);

  std::size_t run_one_( std::unique_lock<mutex_t>& lck);

  std::size_t loop_(std::unique_lock<mutex_t>& lck, bool one);

  void run_wait_( std::unique_lock<mutex_t> & lck);

  struct queue_cmp
  {
    inline bool operator()( delayed_queue::event_t & e1, delayed_queue::event_t & e2 ) const
    {
      return e1.first > e2.first;
    }
  };

private:

  mutable mutex_t          _mutex;
  condition_variable_t     _cond_var;
  queue_t                  _que;
  delayed_queue_t          _delayed_que;
  std::atomic<bool>        _loop_exit;

  std::atomic<size_t> _counter;
  std::atomic<size_t> _safe_counter;

  std::atomic<size_t> _maxsize;
  std::atomic<size_t> _drop_count;
}; // delayed_queue
}


