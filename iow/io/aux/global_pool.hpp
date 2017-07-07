#pragma once
#include <iow/io/aux/data_pool_options.hpp>
#include <iow/io/types.hpp>
#include <functional>

namespace iow{ namespace io{
  
typedef std::function< data_ptr(size_t, size_t) > create_fun;
typedef std::function< void(data_ptr) > free_fun;

class global_pool
{
public:
  static void initialize(data_map_options);
  static create_fun get_create();
  static free_fun get_free();
  static data_ptr create(size_t, size_t);
  static void free(data_ptr d);
};

}}
