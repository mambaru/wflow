#pragma once

#include <iow/io/descriptor/context.hpp>
#include <iow/io/types.hpp>
#include <vector>
#include <memory>

namespace iow{ namespace io{ namespace socket{ namespace dgram{

struct context: public ::iow::io::descriptor::context<io_id_t, data_type, data_ptr>
{
};

}}}}
