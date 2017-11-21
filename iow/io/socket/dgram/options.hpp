#pragma once

#include <iow/io/socket/dgram/context.hpp>
#include <iow/io/descriptor/options.hpp>
#include <iow/io/rw/options.hpp>

namespace iow{ namespace io{ namespace socket{ namespace dgram{

struct options: 
  ::iow::io::descriptor::options<context>,
  ::iow::io::rw::options
{
  typedef ::iow::io::descriptor::options<context> descriptor_options_type;
};

}}}}
