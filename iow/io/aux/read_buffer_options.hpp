#pragma once

#include <iow/io/types.hpp>
#include <string>
#include <memory>
#include <functional>

namespace iow{ namespace io{
  
struct read_buffer_options
{
  typedef std::function< data_ptr(size_t, size_t) > create_fun;
  typedef std::function< void(data_ptr) > free_fun;

  //std::string sep=std::string("\r\n");
  std::string sep;
  size_t bufsize=4096;
  size_t maxbuf=4096*2;
  size_t minbuf=0;
  size_t maxsize=0;
  bool fast_mode = false;
  bool trimsep = true; // Отрезать сепаратор 
  std::function< data_ptr(size_t, size_t) > create;
  std::function< void(data_ptr) > free;
};

}}
