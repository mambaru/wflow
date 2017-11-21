#pragma once

#include <iow/io/socket/ad_initialize.hpp>
#include <iow/io/socket/stream/asio/aspect.hpp>
#include <iow/io/socket/stream/context.hpp>
#include <iow/io/descriptor/aspect.hpp>
#include <iow/io/descriptor/ad_input_handler.hpp>

#include <iow/io/descriptor/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/tags.hpp>
#include <iow/io/rw/tags.hpp>


namespace iow{ namespace io{ namespace socket{ namespace stream{

struct _initialize_;

struct aspect: fas::aspect<
  fas::advice< _initialize_, ::iow::io::socket::ad_initialize>,
  fas::advice< ::iow::io::reader::data::_input_, ::iow::io::descriptor::ad_input_handler>,
  fas::alias< ::iow::io::descriptor::_output_, ::iow::io::writer::_output_>,
  ::iow::io::socket::stream::asio::aspect,
  ::iow::io::descriptor::aspect< context, _initialize_, true >
  //::iow::io::descriptor::aspect< context, ::iow::io::rw::_initialize_, true >
>{};
  
}}}}
