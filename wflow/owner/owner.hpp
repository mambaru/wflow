//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018, 2021
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wflow/owner/owner_handler.hpp>
#include <wflow/owner/callback_handler.hpp>
#include <wflow/mutex.hpp>
#include <memory>
#include <atomic>
#include <memory>
#include <map>

namespace wflow{

class owner
{
public:
  typedef size_t io_id_t;

  typedef std::shared_ptr<int> alive_type;
  typedef std::weak_ptr<int>   weak_type;

  typedef std::function<void()> double_call_fun_t;
  typedef std::function<void()> no_call_fun_t;
  typedef rwlock<std::mutex> mutex_type;

  owner()
    : _alive( std::make_shared<int>(1) )
    , _tracking_flag(false)
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

  void release_tracking(io_id_t io_id)
  {
    if ( _tracking_flag )
    {
      std::lock_guard<mutex_type> lk(_mutex);
      _tracking_map.erase(io_id);
    }
  }

  void enable_tracking(bool value)
  {
    if ( _tracking_flag == value )
      return;

    _tracking_flag = value;
    if (!value)
    {
      std::lock_guard<mutex_type> lk(_mutex);
      _tracking_map.clear();
    }
  }


  std::weak_ptr<int> tracking(io_id_t io_id)
  {
    std::weak_ptr<int> wc;
    std::lock_guard<mutex_type> lk(_mutex);
    auto itr = _tracking_map.find(io_id);
    if ( itr!=_tracking_map.end() )
    {
      ++*(itr->second);
      wc=itr->second;
    }
    else
    {
      wc = _tracking_map.insert( std::make_pair(io_id, std::make_shared<int>(1)) ).first->second;
    }
    return wc;
  }

  template<typename Handler, typename AltHandler>
  owner_handler<
    typename std::remove_reference<Handler>::type,
    typename std::remove_reference<AltHandler>::type
  >
  tracking(io_id_t io_id, Handler&& h, AltHandler&& nh)
  {
    std::weak_ptr<int> wc = this->tracking(io_id);
    return
      owner_handler<
        typename std::remove_reference<Handler>::type,
        typename std::remove_reference<AltHandler>::type
      >(
          std::forward<Handler>(h),
          std::forward<AltHandler>(nh),
          wc
       );
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

  void set_double_call_handler(const double_call_fun_t& dc)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _double_call = dc;
  }

  void set_no_call_handler(const no_call_fun_t& nc)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _no_call = nc;
  }

  size_t tracking_size() const
  {
    read_lock<mutex_type> lk(_mutex);
    return _tracking_map.size();
  }

private:
  mutable alive_type _alive;
  double_call_fun_t _double_call;
  no_call_fun_t _no_call;
  mutable mutex_type _mutex;
  std::atomic_bool _tracking_flag;
  std::map<io_id_t, std::shared_ptr<int> > _tracking_map;
};

}
