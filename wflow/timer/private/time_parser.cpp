#include <wjson/wjson.hpp>
#include "time_parser.hpp"

namespace wflow{

namespace{ namespace detail{

bool make_check(const std::string& strtime, std::string* err);

bool make_delay(const std::string& strtime, time_t* result, std::string* err);

bool make_time(const std::string& strtime, time_t* result, std::string* err);

}}


bool time_parser::is_time(const std::string& strtime)
{
  return std::count( std::begin(strtime), std::end(strtime), ':') == 2;
}

bool time_parser::is_schedule(const std::string& strtime)
{
  return std::count( std::begin(strtime), std::end(strtime), ' ') >= 5;
}

bool time_parser::is_interval(const std::string& strtime)
{
  return !(is_time(strtime) || is_schedule(strtime));
}



bool time_parser::make_duration(const std::string& strtime, duration_t* result, std::string* err)
{
  time_t val = 0;
  bool ret = detail::make_delay(strtime, &val, err);
  if ( ret && result != nullptr )
    *result = std::chrono::seconds(val);
  return ret;
}

bool time_parser::make_time_point(const std::string& strtime, time_point_t* result, std::string* err)
{
  time_t val = 0;
  bool ret = detail::make_time(strtime, &val, err);
  if ( ret && result!=nullptr )
    *result = std::chrono::system_clock::from_time_t(val);
  return ret;
}

bool time_parser::make_cron(const std::string& schedule, cron_t* result, std::string* err)
try
{
  auto crn = cron::make_cron(schedule);
  if ( result != nullptr )
    *result = crn;
  return true;
}
catch (cron::bad_cronexpr const & ex)
{
  if ( err!=nullptr  )
   *err = ex.what();
  return false;
}

time_t time_parser::cron_next(const cron_t& crn)
{
  std::time_t now = std::time(nullptr);
  return cron::cron_next(crn, now);
}


namespace{ namespace detail{

bool make_check(const std::string& strtime, std::string* err)
{
  if ( err!=nullptr && !err->empty())
    return false;
  
  if ( strtime.empty() )
    return false;
  
  return true;
}

bool make_delay(const std::string& strtime, time_t* delay, std::string* err)
{
  if ( delay != nullptr )
    *delay = 0;
  
  if (!make_check( strtime, err) )
    return false;

  time_t now = std::time(nullptr);

  if ( time_parser::is_interval(strtime) )
  {
    typedef wjson::time_interval<time_t, 1>::serializer serializer;
    wjson::json_error e;
    time_t val = 0;
    std::string json="\"" + strtime + "\"";
    serializer()(val, std::begin(json), std::end(json), &e );
    if (e)
    {
      if (err!=nullptr)
        *err = wjson::strerror::message_trace(e, std::begin(json), std::end(json));
      return false;
    }
    if ( delay!=nullptr )
      *delay = val;
    return true;
  }
  
  time_t tp = 0;
  if ( !make_time(strtime, &tp, err ) )
    return false;
  if ( delay!=nullptr )
    *delay = tp - now;
  return true;
}

bool make_time(const std::string& strtime, time_t* tp, std::string* err)
{
  if ( tp != nullptr )
    *tp = std::time(nullptr);
  
  if (!make_check( strtime, err) )
    return false;

  std::time_t now = std::time(nullptr);
  
  if ( time_parser::is_time(strtime) )
  {
    std::tm ptm;
    ::localtime_r(&now, &ptm);
    if ( nullptr ==  strptime(strtime.c_str(), "%H:%M:%S", &ptm) )
    {
      if ( tp!=nullptr ) *tp = 0;
      if ( err!=nullptr ) *err = "Invalid time format: '" + strtime + "'";
      return false;
    }

    constexpr time_t day_in_sec = 3600*24;
    time_t beg_day = (now/day_in_sec) * day_in_sec - ptm.tm_gmtoff;
    time_t ready_time_of_day = ptm.tm_sec + ptm.tm_min*60 + ptm.tm_hour*3600;
    time_t ready_time = beg_day + ready_time_of_day;
    if ( ready_time < now )
      ready_time += day_in_sec;
    if ( tp!=nullptr )
      *tp = ready_time;
    return true;
  }
  else if ( time_parser::is_schedule(strtime) )
  {
    time_parser::cron_t crn;
    if ( !time_parser::make_cron(strtime, &crn, err) )
      return false;
    
    if ( tp!=nullptr )
      *tp = time_parser::cron_next(crn);
    return true;
  }

  time_t delay = 0;
  if ( !make_delay(strtime, &delay, err ) )
    return false;
  if ( tp!=nullptr )
    *tp += delay;
  return true;
  
}

}} // namespace . detail 


} //namespace wflow
