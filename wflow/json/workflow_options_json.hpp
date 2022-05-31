#pragma once

#include <wflow/workflow_options.hpp>
#include <wjson/json.hpp>
#include <wjson/name.hpp>

namespace wflow {

struct workflow_options_basic_json
{
  JSON_NAME(threads)
  JSON_NAME(debug)
  JSON_NAME(maxsize)
  JSON_NAME(wrnsize)
  JSON_NAME(quiet_mode)
  JSON_NAME(overflow_reset)
  JSON_NAME(control_ms)


  typedef wjson::object<
    workflow_options,
    wjson::member_list<
      wjson::member< n_threads, workflow_options, size_t,  &workflow_options::threads >,
      wjson::member< n_wrnsize, workflow_options, size_t, &workflow_options::wrnsize >,
      wjson::member< n_maxsize, workflow_options, size_t, &workflow_options::maxsize >,
      wjson::member< n_quiet_mode, workflow_options, bool, &workflow_options::quiet_mode >,
      wjson::member< n_overflow_reset, workflow_options, bool, &workflow_options::overflow_reset >,
      wjson::member< n_control_ms, workflow_options, time_t, &workflow_options::control_ms, wjson::time_interval_ms<> >,
      wjson::member< n_debug, workflow_options, bool, &workflow_options::debug >
    >,
    wjson::strict_mode
  > type;
  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};

struct workflow_options_json
{
  JSON_NAME(post_delay_ms)
  JSON_NAME(rate_limit)

  typedef wjson::object<
    workflow_options,
    wjson::member_list<
      wjson::base< workflow_options_basic_json >,
      wjson::member< n_post_delay_ms, workflow_options, time_t, &workflow_options::post_delay_ms, wjson::time_interval<time_t, 1000> >,
      wjson::member< n_rate_limit, workflow_options, size_t, &workflow_options::rate_limit >
    >,
    wjson::strict_mode
  > type;

  typedef typename type::target target;
  typedef typename type::serializer serializer;
  typedef typename type::member_list member_list;
};


}
