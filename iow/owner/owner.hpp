//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/owner/owner_handler.hpp>
#include <iow/owner/callback_handler.hpp>
#include <iow/mutex.hpp>
#include <memory>
#include <atomic>

namespace iow{

class owner
{
public:
  typedef std::shared_ptr<int> alive_type;
  typedef std::weak_ptr<int>   weak_type;

  typedef std::function<void()> double_call_fun_t;
  typedef std::function<void()> no_call_fun_t;
  typedef rwlock<spinlock> mutex_type;

  owner() 
  : _alive( std::make_shared<int>(1) ) 
  {
  }

  owner(const owner& ) = delete;
  owner& operator = (const owner& ) = delete;

  owner(owner&& ) = default;
  owner& operator = (owner&& ) = default;

  alive_type alive() const 
  {
    read_lock<mutex_type> lk(_mutex);
    return _alive; 
  }
  
  void reset()
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _alive = std::make_shared<int>(*_alive + 1);
  }


  template<typename Handler, typename AltHandler>
  owner_handler< 
    typename std::remove_reference<Handler>::type, 
    typename std::remove_reference<AltHandler>::type
  >
  wrap(Handler&& h, AltHandler&& nh) const
  {
    read_lock<mutex_type> lk(_mutex);
    return 
      owner_handler<
        typename std::remove_reference<Handler>::type, 
        typename std::remove_reference<AltHandler>::type
      >(
          std::forward<Handler>(h),
          std::forward<AltHandler>(nh),
          std::weak_ptr<int>(_alive)
       )
    ;
  }
  
  template<typename Handler>
  callback_handler< 
    typename std::remove_reference<Handler>::type
  >
  callback(Handler&& h) const
  {
    read_lock<mutex_type> lk(_mutex);
    auto ready = std::make_shared< std::atomic_flag >();
    return 
      callback_handler<
        typename std::remove_reference<Handler>::type
      >(
          std::forward<Handler>(h),
          ready, _double_call, _no_call
       )
    ;
  }

  void set_double_call_handler(double_call_fun_t dc)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _double_call = dc;
  };

  void set_no_call_handler(no_call_fun_t nc)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _no_call = nc;
  };
 
private:
  mutable alive_type _alive;
  double_call_fun_t _double_call;
  no_call_fun_t _no_call;
  mutable mutex_type _mutex;

};

}
