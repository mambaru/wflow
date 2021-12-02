

#include <wflow/queue/bique.hpp>
#include <wflow/queue/delayed_queue.hpp>
#include <wflow/queue/asio_queue.hpp>
#include "private/time_parser.hpp"
#include "timer_manager.hpp"


namespace wflow{

//
// bique
//
#define TIMER_MANAGER(QUE)                                                                                                    \
template<>                                                                                                                      \
timer_manager<QUE>::timer_manager( queue_ptr queue )                                                                          \
  : _queue(queue){}                                                                                                             \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create( duration_t delay, handler h, expires_at expires)                                                \
{                                                                                                                               \
  return this->create_( clock_t::now() + std::chrono::duration_cast<std::chrono::microseconds>(delay), delay, std::move(h), expires );                                           \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create( duration_t delay, async_handler h, expires_at expires)                                          \
{                                                                                                                               \
  return this->create_( clock_t::now() + std::chrono::duration_cast<std::chrono::microseconds>(delay), delay, std::move(h), expires );                                           \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(duration_t start_delay, duration_t delay, handler h, expires_at expires)                         \
{                                                                                                                               \
  return this->create_( clock_t::now() + std::chrono::duration_cast<std::chrono::microseconds>(start_delay), delay, std::move(h), expires );                                     \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(duration_t start_delay, duration_t delay, async_handler h, expires_at expires)                   \
{                                                                                                                               \
  return this->create_( clock_t::now() + std::chrono::duration_cast<std::chrono::microseconds>(start_delay), delay, std::move(h), expires );                                     \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(time_point_t start_time, duration_t delay, handler h, expires_at expires)                        \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(time_point_t start_time, duration_t delay, async_handler h, expires_at expires)                  \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(const std::string& start_time, duration_t delay, handler h, expires_at expires)                  \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(const std::string& start_time, duration_t delay, async_handler h, expires_at expires)            \
{                                                                                                                               \
  return this->create_( start_time, delay, std::move(h), expires );                                                       \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(const std::string& shedule, handler h, expires_at expires)                                           \
{                                                                                                                               \
  return timer_manager_base::create(this->_queue, shedule, std::move(h), expires);                      \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create(const std::string& shedule, async_handler h, expires_at expires)                                     \
{                                                                                                                               \
  return timer_manager_base::create(this->_queue, shedule, std::move(h), expires);                                              \
}                                                                                                                               \
                                                                                                                              \
template<>                                                                                                                      \
template<typename Handler>                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create_(time_point_t start_time, duration_t delay, Handler h, expires_at expires)                       \
{                                                                                                                               \
  return timer_manager_base::create(this->_queue, start_time, delay, std::move(h), expires);                              \
}                                                                                                                               \
                                                                                                                                \
template<>                                                                                                                      \
template<typename Handler>                                                                                                      \
timer_manager<QUE>::timer_id_t                                                                                                \
  timer_manager<QUE>::create_(const std::string& start_time, duration_t delay, Handler h, expires_at expires)                 \
{                                                                                                                               \
  time_point_t tp = clock_t::now();                                                                                             \
  if ( !start_time.empty() )                                                                                                    \
  {                                                                                                                             \
    tp += std::chrono::duration_cast<std::chrono::microseconds>(delay);                                                                                                                \
    if ( !time_parser::make_time_point( start_time, &tp, nullptr ) )                                                                          \
    { }                                                                                                                         \
  }                                                                                                                             \
  return this->create_(tp, delay, std::move(h), expires);                                                                 \
}


TIMER_MANAGER(bique)
TIMER_MANAGER(asio_queue)
TIMER_MANAGER(delayed_queue)

}
