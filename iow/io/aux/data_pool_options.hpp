#pragma once

#include <cstddef>
namespace iow{ namespace io{

struct data_pool_options
{
  size_t poolsize = 1024;
  size_t minbuf = 64;
  size_t maxbuf = 4096*2;
};

struct data_map_options
  : data_pool_options
{
  bool disabled = false;
  size_t dimension = 1024;
};

}}
