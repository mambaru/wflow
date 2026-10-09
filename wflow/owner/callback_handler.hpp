//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015, 2021
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <utility>
#include <memory>
#include <atomic>
#include <functional>

namespace wflow{

// Общее состояние всех копий одного callback.
// no_call вызывается в деструкторе, когда умирает последний shared_ptr —
// без опроса use_count() из ~callback_handler.
struct callback_control
{
  typedef std::function<void()> no_call_fun_t;

  std::atomic<bool> called{false};
  no_call_fun_t no_call;

  explicit callback_control(no_call_fun_t nc) noexcept
    : no_call(std::move(nc))
  {
  }

  ~callback_control()
  {
    if ( no_call!=nullptr && !called.exchange(true) )
      no_call();
  }
};

template<typename H>
struct callback_handler
{
  typedef std::function<void()> double_call_fun_t;
  typedef std::weak_ptr<void> weak_type;
  typedef std::shared_ptr<callback_control> control_ptr;

  callback_handler() = default;
  callback_handler(const callback_handler&) = default;
  callback_handler(callback_handler&&) = default;
  callback_handler& operator=(const callback_handler&) = default;
  callback_handler& operator=(callback_handler&&) = default;

  callback_handler(H&& h, const control_ptr& control, const double_call_fun_t& dc, const weak_type& alive)
    : _handler(  std::forward<H>(h) )
    , _control(control)
    , _double_call(dc)
    , _alive(alive)
  {
  }
  
  template <class... Args>
  auto operator()(Args&&... args)
    ->  typename std::invoke_result< H, Args&&... >::type
  {
    if ( auto p = _alive.lock() )
    {
      if ( !_control->called.exchange(true) )
        return _handler(std::forward<Args>(args)...);
      else if (_double_call!=nullptr)
        _double_call();
    }
    return typename std::invoke_result< H, Args&&... >::type();
  }
  
private:
  H _handler;
  control_ptr _control;
  double_call_fun_t _double_call;
  weak_type _alive;
};

}
