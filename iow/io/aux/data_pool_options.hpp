#pragma once

#include <cstddef>
namespace iow{ namespace io{

struct data_pool_options
{
  size_t poolsize = 1024;
  size_t minbuf = 128;
  size_t maxbuf = 4096;
};

}}
