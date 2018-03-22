

#include "timer_manager.hpp"
#include "bique.hpp"
#include "delayed_queue.hpp"
#include "asio_queue.hpp"


namespace wflow{

//
// bique
//
#define TIMER_MANAGER(bique)                                                                                                    \
template<>                                                                                                                      \
timer_manager<bique>::timer_manager( queue_ptr queue )                                                                          \
  : _queue(queue){}                                                                                                             \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create( duration_t delay, handler h, expires_at expires)                                                \
{                                                                                                                               \
  return this->create_( clock_t::now() + delay, delay, std::move(h), expires );                                           \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create( duration_t delay, async_handler h, expires_at expires)                                          \
{                                                                                                                               \
  return this->create_( clock_t::now() + delay, delay, std::move(h), expires );                                           \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(duration_t start_delay, duration_t delay, handler h, expires_at expires)                         \
{                                                                                                                               \
  return this->create_( clock_t::now() + start_delay, delay, std::move(h), expires );                                     \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(duration_t start_delay, duration_t delay, async_handler h, expires_at expires)                   \
{                                                                                                                               \
  return this->create_( clock_t::now() + start_delay, delay, std::move(h), expires );                                     \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(time_point_t start_time, duration_t delay, handler h, expires_at expires)                        \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(time_point_t start_time, duration_t delay, async_handler h, expires_at expires)                  \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(const std::string& start_time, duration_t delay, handler h, expires_at expires)                  \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(const std::string& start_time, duration_t delay, async_handler h, expires_at expires)            \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(std::string start_time, handler h, expires_at expires)                                           \
{                                                                                                                               \
  return this->create( std::move(start_time), std::chrono::milliseconds(0), std::move(h), expires );                      \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create(std::string start_time, async_handler h, expires_at expires)                                     \
{                                                                                                                               \
  return this->create( std::move(start_time), std::chrono::milliseconds(0), std::move(h), expires );                      \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
template<typename Handler>                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create_(time_point_t start_time, duration_t delay, Handler h, expires_at expires)                       \
{                                                                                                                               \
  return timer_manager_base::create(this->_queue, start_time, delay, std::move(h), expires);                              \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
template<typename Handler>                                                                                                      \
timer_manager<bique>::timer_id_t                                                                                                \
  timer_manager<bique>::create_(const std::string& start_time, duration_t delay, Handler h, expires_at expires)                 \
{                                                                                                                               \
  time_point_t tp = clock_t::now();                                                                                             \
  if ( !start_time.empty() )                                                                                                    \
  {                                                                                                                             \
    tp += delay;                                                                                                                \
    if ( !timer::today_from_string( start_time, tp ) )                                                                          \
    { }                                                                                                                         \
  }                                                                                                                             \
  return this->create_(tp, delay, std::move(h), expires);                                                                 \
}


TIMER_MANAGER(bique)
TIMER_MANAGER(asio_queue)
TIMER_MANAGER(delayed_queue)

}
