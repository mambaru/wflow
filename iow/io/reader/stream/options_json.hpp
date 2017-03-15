#pragma once

#include <iow/io/reader/stream/options.hpp>
#include <iow/io/aux/read_buffer_options_json.hpp>
#include <wjson/json.hpp>

namespace iow{ namespace io{ namespace reader{ namespace stream{

struct options_json
{
  typedef ::iow::io::reader::stream::options  options_type;
  typedef ::iow::io::read_buffer_options_json buffer_json;

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::base<buffer_json>
    >
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}}}
