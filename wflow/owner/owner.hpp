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
#include <map>

namespace wflow{

/// Токен lifetime owner'а (поколение).
/// reset() подменяет shared_ptr → старые wrap/callback видят протухший weak_ptr.
struct alive_token {};

/// Токен lifetime соединения (io_id).
/// Пока shared_ptr жив в map — tracking-wrap вызывает primary.
/// release_tracking уничтожает токен → weak_ptr протухает → alt
/// (например клиент закрыл соединение — нет смысла обрабатывать запрос).
struct tracking_token {};

class owner
{
public:
  typedef size_t io_id_t;

  typedef std::shared_ptr<alive_token> alive_type;
  typedef std::weak_ptr<void>  weak_type;
  typedef std::shared_ptr<tracking_token> tracking_token_ptr;

  typedef std::function<void()> double_call_fun_t;
  typedef std::function<void()> no_call_fun_t;
  typedef rwlock<std::mutex> mutex_type;

  owner()
    : _alive( std::make_shared<alive_token>() )
    , _tracking_flag(false)
  {
  }

  owner(const owner& ) = delete;
  owner& operator = (const owner& ) = delete;

  owner(owner&& other) = delete;
  owner& operator = (owner&& other) = delete;

  alive_type alive() const
  {
    read_lock<mutex_type> lk(_mutex);
    return _alive;
  }

  void reset()
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _alive = std::make_shared<alive_token>();
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
          weak_type(_alive)
       )
    ;
  }

  /// Снять токен соединения. Вызывать при закрытии клиента/сокета, иначе map растёт.
  void release_tracking(io_id_t io_id)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _tracking_flag )
      _tracking_map.erase(io_id);
  }

  /// Включить/выключить per-io_id токены. Выключение очищает map.
  void enable_tracking(bool value)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( _tracking_flag == value )
      return;

    _tracking_flag = value;
    if (!value)
      _tracking_map.clear();
  }


  /// Токен для io_id (при выключенном tracking — общий alive owner'а).
  weak_type tracking(io_id_t io_id)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( !_tracking_flag )
      return _alive;

    auto itr = _tracking_map.find(io_id);
    if ( itr!=_tracking_map.end() )
      return itr->second;

    return _tracking_map.insert(
      std::make_pair(io_id, std::make_shared<tracking_token>())
    ).first->second;
  }

  template<typename Handler, typename AltHandler>
  owner_handler<
    typename std::remove_reference<Handler>::type,
    typename std::remove_reference<AltHandler>::type
  >
  tracking(io_id_t io_id, Handler&& h, AltHandler&& nh)
  {
    weak_type wc = this->tracking(io_id);
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
    auto control = std::make_shared<callback_control>(_no_call);
    return
      callback_handler<
        typename std::remove_reference<Handler>::type
      >(
          std::forward<Handler>(h),
          control, _double_call,
          weak_type(_alive)
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
  bool _tracking_flag;
  std::map<io_id_t, tracking_token_ptr> _tracking_map;
};

}
