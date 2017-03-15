#pragma once

#include <iow/io/socket/dgram/context.hpp>
#include <iow/io/descriptor/options.hpp>
#include <iow/io/stream/options.hpp>

namespace iow{ namespace io{ namespace socket{ namespace dgram{

struct options: 
  ::iow::io::descriptor::options<context>,
  ::iow::io::stream::options
{
};

}}}}
