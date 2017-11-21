//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/logger/logger.hpp>
#include <utility>
#include <memory>
#include <atomic>

namespace iow{

template<typename H>
struct callback_handler
{
  typedef std::function<void()> double_call_fun_t;
  typedef std::function<void()> no_call_fun_t;
  
  typedef std::shared_ptr< std::atomic_flag > ready_ptr;

  callback_handler() = default;
  callback_handler(const callback_handler&) = default;
  callback_handler(callback_handler&&) = default;
  callback_handler& operator=(const callback_handler&) = default;
  callback_handler& operator=(callback_handler&&) = default;

  ~callback_handler()
  {
    if ( _ready!=nullptr && _ready.use_count()==1 && _no_call!=nullptr && !_ready->test_and_set() )
      _no_call();
  }
  
  callback_handler(H&& h, const ready_ptr& ready, const double_call_fun_t& dc, const no_call_fun_t& nc)
    : _handler(  std::forward<H>(h) )
    , _ready(ready)
    , _double_call(dc)
    , _no_call(nc)
  {
  }
  
  template <class... Args>
  auto operator()(Args&&... args)
    ->  typename std::result_of< H(Args&&...) >::type
  {
    if ( !_ready->test_and_set() )
      return _handler(std::forward<Args>(args)...);
    else if (_double_call!=nullptr)
      _double_call();
    return typename std::result_of< H(Args&&...) >::type();
  }
  
private:
  H _handler;
  ready_ptr _ready;
  double_call_fun_t _double_call;
  no_call_fun_t _no_call;
};

}
