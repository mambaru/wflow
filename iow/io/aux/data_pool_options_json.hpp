#pragma once

#include <iow/io/aux/data_pool_options.hpp>
#include <wjson/json.hpp>
namespace iow{ namespace io{

struct data_pool_options_json
{
  typedef data_pool_options options_type;
  //JSON_NAME(disabled)
  JSON_NAME(poolsize)
  JSON_NAME(minbuf)
  JSON_NAME(maxbuf)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      //::wjson::member< n_disabled,  options_type, bool,      &options_type::disabled >,
      ::wjson::member< n_poolsize,  options_type, size_t,      &options_type::poolsize >,
      ::wjson::member< n_maxbuf,    options_type, size_t,      &options_type::maxbuf  >,
      ::wjson::member< n_minbuf,    options_type, size_t,      &options_type::minbuf  >
    >,
    ::wjson::strict_mode
  > type;
  
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

struct data_map_options_json
{
  typedef data_map_options options_type;
  JSON_NAME(disabled)
  JSON_NAME(dimension)

  typedef ::wjson::object<
    options_type,
    ::wjson::member_list<
      ::wjson::member< n_disabled,   options_type, bool,   &options_type::disabled  >,
      ::wjson::member< n_dimension,  options_type, size_t, &options_type::dimension >
    >,
    ::wjson::strict_mode
  > type;
  
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

}}
