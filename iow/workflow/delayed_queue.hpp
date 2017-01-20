#pragma once 

#include <chrono>
#include <atomic>
#include <queue>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace iow {

class delayed_queue
{
  struct queue_cmp;

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

  delayed_queue(size_t maxsize);

  virtual ~delayed_queue ();
  
  void set_maxsize(size_t maxsize);

  void reset();

  std::size_t run();
  
  std::size_t run_one();

  std::size_t poll_one();

  void stop();
  
  bool post( function_t f, function_t drop );
  
  bool post_at(time_point_t time_point, function_t f, function_t drop);

  bool delayed_post(duration_t duration, function_t f, function_t drop);
  
  std::size_t size() const;
  std::size_t dropped() const;

  bool work() { return false;}
private:
  bool check_(function_t drop);

  std::size_t size_() const;

  void push_at_(time_point_t time_point, function_t f);
  
  std::size_t poll_one_( std::unique_lock<mutex_t>& lck);

  std::size_t run_one_( std::unique_lock<mutex_t>& lck);

  std::size_t loop_(std::unique_lock<mutex_t>& lck, bool one);

  void run_wait_( std::unique_lock<mutex_t> & lck);

  struct queue_cmp
  {
    inline bool operator()( delayed_queue::event_t & e1, delayed_queue::event_t & e2 )
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

  size_t                   _maxsize = 0;
  size_t                   _drop_count = 0;
}; // delayed_queue
}


