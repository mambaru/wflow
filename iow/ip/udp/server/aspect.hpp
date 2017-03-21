#pragma once

#include <iow/ip/udp/server/options.hpp>
#include <iow/io/reader/asio/aspect.hpp>
#include <iow/io/writer/asio/aspect.hpp>
#include <iow/io/rw/aspect.hpp>
#include <iow/io/basic/aspect.hpp>
#include <iow/io/descriptor/tags.hpp>
#include <iow/io/socket/dgram/aspect.hpp>
#include <fas/aop.hpp>
#include <mutex>
#include <vector>

namespace iow{ namespace ip{ namespace udp{ namespace server{

struct aspect : fas::aspect<
    fas::type< ::iow::io::descriptor::_descriptor_type_, ::iow::asio::ip::udp::socket >,
    fas::type< ::iow::io::_options_type_, options >,
    ::iow::io::socket::dgram::aspect,
    ::iow::io::reader::asio::aspect,
    ::iow::io::writer::asio::aspect,
    ::iow::io::rw::aspect,
    ::iow::io::basic::aspect< std::recursive_mutex >::advice_list
>{};
  
}}}}
