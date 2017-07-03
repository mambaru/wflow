#pragma once

#include <iow/ip/tcp/connection/options.hpp>
#include <iow/io/rw/options_json.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace ip{ namespace tcp{ namespace connection{
  
struct options_json
{
  typedef ::wjson::object<
    options,
    ::wjson::member_list<
      ::wjson::base< ::iow::io::rw::options_json >
    >,
    ::wjson::strict_mode
  > type;
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};


}}}}

