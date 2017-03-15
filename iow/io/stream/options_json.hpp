#pragma once

#include <iow/io/stream/options.hpp>
#include <iow/io/reader/stream/options_json.hpp>
#include <iow/io/writer/stream/options_json.hpp>
#include <wjson/json.hpp>

namespace iow{ namespace io{ namespace stream{
  
struct options_json
{
  typedef options options_type;
  typedef typename options_type::reader_options reader_options;
  typedef typename options_type::writer_options writer_options;
  typedef ::iow::io::reader::stream::options_json reader_options_json;
  typedef ::iow::io::writer::stream::options_json writer_options_json;
  
  JSON_NAME(reader)
  JSON_NAME(writer)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
        ::wjson::member<n_reader, options_type, reader_options, &options_type::reader, reader_options_json >,
        ::wjson::member<n_writer, options_type, writer_options, &options_type::writer, writer_options_json >
    >
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}}
