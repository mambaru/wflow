//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/logger/logger.hpp>
#include <utility>
#include <memory>


namespace iow{

template<typename R, typename ... Args>
struct callback_handler
{
  typedef std::function< R(Args...) > function_t;
  typedef std::weak_ptr<int> weak_type;

  callback_handler() = default;

  callback_handler(function_t&& h, function_t&& nh,  const weak_type& alive)
    : _handler(  std::forward<function_t>(h) )
    , _alt_handler(  std::forward<function_t>(nh) )
    , _alive(alive)
  {
  }
  
  R operator()(Args&&... args)
  {
    if ( auto p = _alive.lock() )
    {
      return _handler(std::forward<Args>(args)...);
    }
    
    return _alt_handler!=nullptr
           ? _alt_handler(std::forward<Args>(args)...)
           : R();

  }
private:
  function_t _handler;
  function_t _alt_handler;
  weak_type _alive;
};

template<typename H>
struct owner_handler< H, std::nullptr_t >
{
  typedef std::weak_ptr<int> weak_type;

  owner_handler() = default;

  owner_handler(H&& h, std::nullptr_t, const weak_type& alive)
    : _handler(  std::forward<H>(h) )
    , _alive(alive)
  {
  }
  
  template <class... Args>
  auto operator()(Args&&... args)
    ->  typename std::result_of< H(Args&&...) >::type
  {
    if ( auto p = _alive.lock() )
    {
      return _handler(std::forward<Args>(args)...);
    }
    return typename std::result_of< H(Args&&...) >::type();
  }
private:
  H _handler;
  weak_type _alive;
};

}
