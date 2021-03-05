//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2016
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once 

namespace wflow{
  
class empty_mutex
{

public:
  empty_mutex(){}

  empty_mutex(const empty_mutex&) = delete;
  empty_mutex& operator=(const empty_mutex&) = delete;

  static void lock(){}
    
  static void unlock(){}

  static void lock_r() {}
  
  static void unlock_r() {}
};

  
}
