//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <iow/logger/log_writer.hpp>
#include <iow/io/types.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace iow{

class logstream
{
public:
  ~logstream()
  {
    bool ready = false;
    if ( writer_ )
    {
      ready = writer_(_name, _type, _ss.str());
    }
    
    if ( !ready )
    {
#ifndef IOW_DISABLE_CLOG
      std::clog << _name << " " << _type << " " << _ss.str();
#endif
    }
  }
  
  logstream(const logstream& ll) = delete;
  logstream& operator = (logstream& ll) = delete;
  
  logstream(logstream&& ll)
    : _name(std::move(ll._name))
    , _type(std::move(ll._type))
    , _ss()
    , writer_(std::move(ll.writer_))
  {
  }

  logstream& operator = (logstream&& ll)
  {
    _name = std::move(ll._name);
    _type = std::move(ll._type);
    _ss.clear();
    writer_ = std::move(ll.writer_);
    return *this;
  }

  
  logstream(const std::string& name, const std::string& type, const log_writer& writer)
    : _name(name)
    , _type(type)
    , writer_(writer)
  {
  }

  logstream(std::string&& name, std::string&& type, const log_writer& writer)
    : _name( std::move(name) )
    , _type( std::move(type) )
    , writer_(writer)
  {
  }

  std::string str() const
  {
    return _ss.str();
  }

  template<typename V>
  std::stringstream& operator << ( const V&  v)
  {
    _ss<<v;
    return _ss;
  }

private:
  std::string _name;
  std::string _type;
  std::stringstream _ss;
  log_writer writer_;
};

}
