#pragma once

#include <iow/io/writer/stream/options.hpp>
#include <iow/io/aux/write_buffer_options_json.hpp>

namespace iow{ namespace io{ namespace writer{ namespace stream{

struct options_json
{
  typedef ::iow::io::writer::stream::options  options_type;
  typedef ::iow::io::write_buffer_options_json buffer_json;

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
