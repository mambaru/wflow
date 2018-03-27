#pragma once

#include <fas/testing.hpp>
#include <wflow/delayed_queue.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <cmath>
#include <condition_variable>

#define DELAY_MS 200

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
  });
  if (!res)
    t << fail("delayed_post FAIL!");
  
  dq.run_one();
  time_t ms = duration_cast<milliseconds>(finish-start).count();
  t << message("time: ") << ms;
  t << equal<assert>(ms, DELAY_MS) << "delay fail. ms=" << ms << " should be " << DELAY_MS;
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
  });
  dq.post([&t, &finish](){
    finish = high_resolution_clock::now();
    t << message("ready post");
  });
  t << message("run_one: ") << dq.run_one();
  t << message("run_one: ") << dq.run_one();
  time_t ms = duration_cast<milliseconds>(finish-start).count();
  t << message("time: ") << ms;
  t << equal<assert>(ms, DELAY_MS) << "delay fail. ms=" << ms << " should be " << DELAY_MS;
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
    dq.post([&count](){ ++count; });
    time_t& tm = time_chk[i];
    dq.delayed_post(milliseconds(time_ms[i]), [i, start, &count, &tm]()
    { 
      ++count; 
      tm = duration_cast<milliseconds>( high_resolution_clock::now() - start ).count();
    });
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
  for (int i=0 ; i < 5; ++i)
  {
    t << equal<expect>(time_ms[i], time_chk[i]) << "delay fail. ms=" << time_chk[i] << " should be " << time_ms[i];
  }
}

template<typename T, typename Q>
inline void delayed_unit4(T& t, Q& dq)
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  t << message("DEBUG: unit start");
  std::vector<time_t> times(10);
  high_resolution_clock::time_point start;
  for (size_t i = 0; i < 10 ; ++i)
  {
    // пуляем с задержкой 50, 100, 150 ... милисекунд
    dq.delayed_post( milliseconds(50*(i+1)),[&t, &start, &times, i]()
    {
      time_t& ms = times[i];
      ms = duration_cast<milliseconds>( high_resolution_clock::now() - start ).count();
    });
  }
  
  std::condition_variable cv;
  
  volatile bool notify = false;
  for (int i = 0; i < 4 ; ++i)
  {
    std::thread( [&](){
      std::mutex m;
      std::unique_lock< std::mutex> lck(m);
      cv.wait(lck, [&]{ return !notify;});
      dq.run();
    } ).detach();
  }
  std::this_thread::sleep_for( milliseconds(5) );
  start = high_resolution_clock::now();
  notify = true;
  cv.notify_all();
  std::this_thread::sleep_for( milliseconds(650) );
  dq.stop();
  std::this_thread::sleep_for( milliseconds(10) );
  for (size_t i = 0; i < 10 ; ++i)
  {
    time_t res = static_cast<time_t>(50*(i+1)) - times[i] - 5;
    t << is_true<expect>( std::abs(res) < 2 ) << " time error ( " << 50*(i+1) << "-" << times[i] << " > 1 )" << notify ;
  }
}
