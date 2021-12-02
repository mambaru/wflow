#pragma once

#include <chrono>
#include <memory>
#include <functional>
#include "croncpp/croncpp.h"

namespace wflow{

class time_parser
{
public:
  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef cron::cronexpr cron_t;

  static bool is_time(const std::string& strtime);
  static bool is_interval(const std::string& strtime);
  static bool is_schedule(const std::string& strtime);
  
  static bool make_time_point(const std::string& strtime, time_point_t* result, std::string* err);
  static bool make_duration(const std::string& strtime, duration_t* result, std::string* err);
  static bool make_cron(const std::string& strtime, cron_t* result, std::string* err);
  
  static time_t cron_next(const cron_t& crn);
};

}
