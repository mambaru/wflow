#pragma once

#include <iow/io/socket/stream/context.hpp>
#include <iow/io/descriptor/options.hpp>
#include <iow/io/rw/options.hpp>

namespace iow{ namespace io{ namespace socket{ namespace stream{

struct options: 
  ::iow::io::descriptor::options<context>,
  ::iow::io::rw::options
{
};

}}}}
