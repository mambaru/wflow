#pragma once

#include <iow/ip/tcp/connection/options_json.hpp>
#include <iow/ip/tcp/acceptor/options_json.hpp>
#include <iow/ip/tcp/server/options.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace server{
 
template<
  typename ServerOptions = ::iow::ip::tcp::server::options<>, 
  typename AcceptorJson = iow::ip::tcp::acceptor::options_json<>
>
struct options_json
{
  JSON_NAME(threads)
  
  typedef ::wjson::object<
    ServerOptions,
    ::wjson::member_list<
      ::wjson::member<n_threads, ServerOptions, int, &ServerOptions::threads>, //< перенести в общий серевер
      ::wjson::base< AcceptorJson >
    >
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}}}

