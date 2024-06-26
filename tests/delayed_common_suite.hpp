#pragma once

#include <fas/testing.hpp>
#include <wflow/queue/delayed_queue.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <cmath>
#include <condition_variable>

#define DELAY_MS 200

namespace {
  
template<typename T>
time_t get_accuracy(T& t)
{
  if ( t.get_argc() < 2 )
    return 50;
  
  return std::atoi( t.get_arg(1).c_str() );
}

template<typename T, typename Q>
inline void delayed_unit1(T& t, Q& dq)
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  
  system_clock::time_point start = system_clock::now();
  system_clock::time_point finish = start;
  auto res = dq.delayed_post( milliseconds(DELAY_MS), [&t, &finish](){
    t << message("delayed_post READY!");
    finish = system_clock::now();
  }, nullptr);
  if (!res)
    t << fail("delayed_post FAIL!") << FAS_FL;
  
  dq.run_one();
  time_t ms = duration_cast<milliseconds>(finish-start).count();
  t << message("time: ") << ms;
  time_t accuracy = get_accuracy(t);
  t << greater_equal<assert>(ms, DELAY_MS) << FAS_FL;
  t << less_equal<assert>(ms, DELAY_MS + accuracy) <<  FAS_FL;
}


template<typename T, typename Q>
inline void delayed_unit2(T& t, Q& dq)
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  
  high_resolution_clock::time_point start = high_resolution_clock::now();
  high_resolution_clock::time_point finish = start;
  dq.delayed_post( milliseconds(DELAY_MS), [&t, &finish](){
    finish = high_resolution_clock::now();
    t << message("ready delayed_post");
  }, nullptr);
  dq.post([&t, &finish](){
    finish = high_resolution_clock::now();
    t << message("ready post");
  }, nullptr);
  t << message("run_one: ") << dq.run_one();
  t << message("run_one: ") << dq.run_one();
  time_t ms = duration_cast<milliseconds>(finish-start).count();
  t << message("time: ") << ms;
  time_t accuracy = get_accuracy(t);
  t << greater_equal<assert>(ms, DELAY_MS) << FAS_FL;
  t << less_equal<assert>(ms, DELAY_MS + accuracy) <<  FAS_FL;
}

template<typename T, typename Q>
inline void delayed_unit3(T& t, Q& dq)
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  
  time_t time_ms[] = {100, 200, 300, 400, 500};
  time_t time_chk[] = { 0,   0,   0,   0,   0};
  std::atomic<size_t> count;
  count = 0;
  auto start = high_resolution_clock::now();
  for (int i=0 ; i < 5; ++i)
  {
    dq.post([&count]() noexcept{ ++count; }, nullptr);
    time_t& tm = time_chk[i];
    dq.delayed_post(milliseconds(time_ms[i]), [start, &count, &tm]()
    { 
      ++count; 
      tm = duration_cast<milliseconds>( high_resolution_clock::now() - start ).count();
    }, nullptr);
  }
  t << message( "start thread... ") << count;
  
  std::thread th([&dq, &t, &start](){
    t << message( "thread started ") ;
    start = high_resolution_clock::now();
    dq.run();
  });
  t << message( "while count ... ") << count;
  while (count < 10)
    std::this_thread::sleep_for( milliseconds(10)  );
    
  t << message( "stop... ") << count ;
  dq.stop();
  th.join();
  
  t << equal<expect, int>(count, 10) << "count=" << count << " should be 10";
  time_t accuracy = get_accuracy(t) + 10 ;
  for (int i=0 ; i < 5; ++i)
  {
    //t << equal<expect>(time_ms[i]/10, time_chk[i]/10) << "delay fail. ms=" << time_chk[i] << " should be " << time_ms[i];
    t << greater_equal<assert>(time_chk[i], time_ms[i]) << FAS_FL;
    t << less_equal<assert>(time_chk[i], time_ms[i] + accuracy) <<  FAS_FL;
  }
}

}
