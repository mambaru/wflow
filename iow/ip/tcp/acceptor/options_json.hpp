#pragma once

#include <iow/ip/tcp/acceptor/options.hpp>
#include <iow/ip/tcp/connection/options_json.hpp>
#include <iow/ip/tcp/connection/connection.hpp>
#include <iow/io/acceptor/options_json.hpp>
#include <wjson/json.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace acceptor{

template<
  typename AcceptorOptions = ::iow::ip::tcp::acceptor::options<>,
  typename ConnectionOptionsJson = ::iow::ip::tcp::connection::options_json
>
using options_json = ::iow::io::acceptor::options_json<AcceptorOptions, ConnectionOptionsJson >;

}}}}

