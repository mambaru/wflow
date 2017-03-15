#pragma once

#include <iow/io/aux/data_pool.hpp>
#include <iow/io/descriptor/context.hpp>

#include <vector>
#include <memory>

namespace iow{ namespace io{ namespace socket{ namespace stream{

struct context: public ::iow::io::descriptor::context<io_id_t, data_type, data_ptr>
{
};

}}}}
