#pragma once

#include <iow/io/aux/data_pool.hpp>
#include <iow/io/descriptor/context.hpp>

#include <vector>
#include <memory>

namespace iow{ namespace io{ namespace socket{ namespace stream{

namespace base{
  using context = ::iow::io::descriptor::context<
    size_t, 
    std::vector<char>, 
    std::unique_ptr<std::vector<char> >
  >;
}

struct context: base::context
{
  typedef base::context::data_type data_type;
};

}}}}
