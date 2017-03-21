#pragma once

#include <iow/io/socket/stream/asio/aspect.hpp>
#include <iow/io/socket/stream/context.hpp>
#include <iow/io/descriptor/aspect.hpp>
#include <iow/io/descriptor/ad_incoming_handler.hpp>

#include <iow/io/descriptor/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/tags.hpp>
#include <iow/io/rw/tags.hpp>


namespace iow{ namespace io{ namespace socket{ namespace stream{

struct aspect: fas::aspect<
  fas::advice< ::iow::io::reader::data::_incoming_, ::iow::io::descriptor::ad_incoming_handler>,
  fas::alias< ::iow::io::descriptor::_outgoing_, ::iow::io::writer::_output_>,
  ::iow::io::socket::stream::asio::aspect,
  ::iow::io::descriptor::aspect< context, ::iow::io::rw::_initialize_, true >
>{};
  
}}}}
