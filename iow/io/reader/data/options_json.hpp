#pragma once

#include <iow/io/reader/data/options.hpp>
#include <iow/io/aux/read_buffer_options_json.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace iow{ namespace io{ namespace reader{ namespace data{

struct options_json
{
  typedef ::iow::io::reader::data::options  options_type;
  typedef ::iow::io::read_buffer_options_json buffer_json;

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::base<buffer_json>
    >,
    ::wjson::strict_mode
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}}}
