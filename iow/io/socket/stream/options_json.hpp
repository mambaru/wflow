#pragma once

#include <iow/io/socket/stream/options.hpp>
#include <iow/io/rw/options_json.hpp>
#include <wjson/json.hpp>

namespace iow{ namespace io{ namespace socket{ namespace stream{
  
struct options_json
{
  typedef options options_type;
  typedef options::descriptor_options_type desc_options_type;
  JSON_NAME(receive_buffer_size)
  JSON_NAME(send_buffer_size)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
        ::wjson::base< iow::io::rw::options_json >,
        ::wjson::member<n_receive_buffer_size, desc_options_type, size_t, &desc_options_type::receive_buffer_size>,
        ::wjson::member<n_send_buffer_size, desc_options_type, size_t, &desc_options_type::send_buffer_size >
    >,
    ::wjson::strict_mode
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}}}
