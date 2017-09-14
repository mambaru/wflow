//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/owner/owner_handler.hpp>
#include <memory>
#include <atomic>

namespace iow{

class owner
{
public:

  typedef std::shared_ptr<int> alive_type;
  typedef std::weak_ptr<int>   weak_type;

  owner() 
  : _alive( std::make_shared<int>(1) ) 
  {
  }

  owner(const owner& ) = delete;
  owner& operator = (const owner& ) = delete;

  owner(owner&& ) = default;
  owner& operator = (owner&& ) = default;

  alive_type& alive() { return _alive; }
  const alive_type& alive() const { return _alive; }
  
  void reset()
  {
    _alive = std::make_shared<int>(*_alive + 1);
  }

  /*
  template<typename ...Args>
  owner_handler< 
    std::function<void(Args...)>, 
    std::function<void(Args...)>
  >
  wrap(std::function<void(Args...)>&& h, std::function<void(Args...)>&& nh) const
  {
    return 
      owner_handler<
        std::function<void(Args...)>, 
        std::function<void(Args...)>
      >(
          std::forward<std::function<void(Args...)>>(h),
          std::forward<std::function<void(Args...)>>(nh),
          std::weak_ptr<int>(_alive)
       )
    ;
  }

    template<typename ...Args>
  owner_handler< 
    std::function<void(Args...)>, 
    std::nullptr_t
  >
  wrap(std::function<void(Args...)>&& h, std::nullptr_t) const
  {
    return 
      owner_handler<
        std::function<void(Args...)>, 
        std::nullptr_t
      >(
          std::forward<std::function<void(Args...)>>(h),
          std::nullptr_t(),
          std::weak_ptr<int>(_alive)
       )
    ;
  }
 */

  template<typename Handler, typename AltHandler>
  owner_handler< 
    typename std::remove_reference<Handler>::type, 
    typename std::remove_reference<AltHandler>::type
  >
  wrap(Handler&& h, AltHandler&& nh) const
  {
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
  
  template<typename R, typename ... Args>
  std::function<R(Args...)> callback( std::function<R(Args...)>, std::function<R(Args...)> )
  {
    return nullptr;
  }

  void set_callback_check(std::function<void()> value)
  {
    _callback_check = value;
  };

  // TODO: wrap_strong и wrap_strong2 строго на один вызов
  // Если обработчик уничтожаеться без вызова или вызываеться повторно то ошибка (abort)
  // В _alive можно сделать счетчик еще невызванных (только для strong)
  // TODO:enable_callback_check(true) в конфиге core, флаг в глобале, устанавливаеться в domain_object

private:
  std::function<void()> _callback_check;
  mutable alive_type _alive;
};

}
