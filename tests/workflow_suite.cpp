
#include <fas/testing.hpp>
#include <wflow/task/task_manager.hpp>
#include <wflow/workflow.hpp>
#include <fas/system/memory.hpp>
#include <chrono>
#include <memory>
#include <set>

namespace {
  
template<typename T>
time_t get_accuracy(T& t)
{
  if ( t.get_argc() < 2 )
    return 0;
  
  return std::atoi( t.get_arg(1).c_str() );
}

UNIT(workflow1, "")
{
  using namespace ::fas::testing;
  t << flush;
  wflow::asio::io_service io;
  wflow::workflow_options opt;
  opt.id="workflow1";
  opt.maxsize = 3;
  opt.threads = 1;
  opt.use_asio = false;
  wflow::task_manager queue(io, opt);
  queue.start();
  std::mutex m;
  int counter = 0;
  queue.post([&t, &counter, &m](){
    std::lock_guard<std::mutex> lk(m);
    ++counter;
    t << message("simple post");
    t << flush;
  }, [&](){ t << fatal("DROP"); ++counter;});
  
  queue.get_timer_manager()->create(std::chrono::milliseconds(400), [&t, &counter, &m](){
    std::lock_guard<std::mutex> lk(m);
    ++counter;
    t << message("timer 400ms");
    t << flush;
    return true;
  });
  queue.delayed_post( std::chrono::milliseconds(600), [&t, &counter, &m](){
    std::lock_guard<std::mutex> lk(m);
    ++counter;
    t << message("delayed post 600ms");
    t << flush;
  }, nullptr);
  
  for (int i =0 ; i < 3 ; ++i)
  {
    queue.delayed_post( std::chrono::milliseconds(300 + i*300), [&t, &counter, i, &m](){
      std::lock_guard<std::mutex> lk(m);
      ++counter;
      t << message("delayed post N") <<  i << " = " << 300 + i*300 << "ms";
      t << flush;
    }, nullptr);
  }
  
  t << flush;
  t << message("sleep...");
  t << flush;
  sleep(2);
  t << message("flush...");
  t << flush;
  queue.stop();
  time_t accuracy = get_accuracy(t);
  if ( accuracy == 0 )
    t << equal< assert,int >( counter, 7 ) << FAS_FL ;
  
}

UNIT(workflow2, "5 сообщений, одно 'теряется' и одно остаеться в очереди")
{
  using namespace ::fas::testing;
  ::wflow::asio::io_service io;  
  ::wflow::workflow_options opt;
  std::atomic<int> counter(0);
  std::atomic<int> dropped(0);
  
  opt.id="workflow2";
  opt.maxsize = 4; 
  opt.threads = 0;
  ::wflow::workflow wfl(io, opt);
  wfl.start();

  for (int i =0 ; i < 5; i++)
  {
    wfl.post(
      [&t, i, &counter, &io]()
      {
        t << message("for 0..5 i=") << i;
        t << flush;
        ++counter;
        if ( counter == 3 )
          io.stop();
      },
      [&dropped, &t]()
      {
        t << warning("DROP");
        ++dropped; 
      }
    );
    
  }
  
  io.run();

  t << equal<expect, size_t>(counter, 3) << FAS_FL;
  t << equal<expect, size_t>(wfl.dropped(), 1) << FAS_FL;
  t << equal<expect, size_t>(dropped, 1) << FAS_FL;
  t << equal<expect, size_t>(wfl.timer_count(), 0) << FAS_FL;
  t << equal<expect, size_t>(wfl.unsafe_size(), 1) << FAS_FL;
}

UNIT(workflow3, "control handler")
{
  using namespace ::fas::testing;
  ::wflow::asio::io_service io;
  ::wflow::asio::io_service::work wrk(io);    
  ::wflow::workflow_options opt;
  std::atomic<int> counter(0);
  opt.threads = 1;
  opt.control_ms = 100;
  opt.control_handler = [&]()->bool{
    if ( counter == 3 )
    {
      t << message("STOP");
      io.stop();
    }
    return true;
  };
  
  ::wflow::workflow wfl(io, opt);
  wfl.start();

  for (int i =0 ; i < 5; i++)
  {
    wfl.post(
      std::chrono::milliseconds(500*(i+1) ), 
      [&t, i, &counter]()
      {
        t << message("for 0..5 i=") << i;
        ++counter;
      }
    );
  }
  // 5 + 1 control_timer 
  t << equal<expect, size_t>(wfl.safe_size(), 6 ) << FAS_FL;
  t << equal<expect, size_t>(wfl.timer_count(), 1) << FAS_FL;
  
  io.run();
  wfl.stop();

  t << equal<expect, size_t>(counter, 3) << FAS_FL;
  t << equal<expect, size_t>(wfl.dropped(), 0) << FAS_FL;
  t << equal<expect, size_t>(wfl.unsafe_size(), 0) << FAS_FL;
  // таймер + две отложенные задачи 
  t << equal<expect, size_t>(wfl.safe_size(), 3) << FAS_FL;
}

struct foo
{
  struct request
  {
    typedef std::unique_ptr<request> ptr; 
  };

  struct response
  {
    typedef std::unique_ptr<response> ptr;
    typedef std::function< void(ptr) > handler;
  };
  void method( request::ptr, response::handler h) { h( std::unique_ptr<response>( new response() ) ); }
};

UNIT(requester1, "")
{
  using namespace ::fas::testing;
  using namespace std::chrono;
  auto f = std::make_shared<foo>();
  wflow::asio::io_service ios;
  wflow::workflow_options wo;
  wo.threads = 0;
  wflow::workflow flw(ios, wo);
  wflow::workflow::timer_id_t id;
  auto start = high_resolution_clock::now();
  auto finish = start;
  id = flw.create_requester< foo::request, foo::response >(
    milliseconds(0),
    milliseconds(1000), 
    [f](foo::request::ptr req, foo::response::handler callback)
    {
      f->method(std::move(req), callback);
      return true;
    },
    [&ios, &finish]( foo::response::ptr ) -> foo::request::ptr
    {
      finish = high_resolution_clock::now();
      ios.stop();
      return foo::request::ptr( new foo::request() ); 
      //return std::make_unique<foo::request>(); // travis-ci clang++5 ws c++14: member named 'make_unique' in namespace 'std'
    }
  );
  t << message("requester: ") << id;
  flw.start();
  ios.run();
  auto interval = duration_cast<milliseconds>(finish - start).count();
//  t << equal<expect>(interval, 0) << FAS_FL;
  time_t accuracy = get_accuracy(t);
  t << less_equal<assert>(interval, accuracy) <<  FAS_FL;


}

UNIT(rate_limit, "")
{
  using namespace ::fas::testing;
  using namespace std::chrono;

  wflow::asio::io_service ios;
  wflow::workflow_options wo;
  wo.threads = 0;
  wo.rate_limit = 100;
  wflow::workflow flw(ios, wo);
  flw.get_timer_manager(); // cppcheck fix
  flw.start();

  auto start = high_resolution_clock::now();
  auto finish = start;
  
  size_t counter = 0;
  for (int i = 0; i != 200; i++)
    flw.post([&](){++counter;});
  ios.run();
  finish = high_resolution_clock::now();
  t << equal<expect, size_t>(counter, 200) << FAS_FL;
  t << equal<expect, size_t>(2, duration_cast<seconds>(finish - start).count()) << FAS_FL;
  time_t accuracy = get_accuracy(t);
  t << less<expect, size_t>(1900, duration_cast<milliseconds>(finish - start).count() + accuracy ) << FAS_FL;
  t << greater<expect, size_t>(2100 + accuracy, duration_cast<milliseconds>(finish - start).count()) << FAS_FL;
  
  t << message("CXX_STANDARD: ") << __cplusplus;
}

UNIT(overflow_reset, "")
{
  using namespace ::fas::testing;
  using namespace std::chrono;
  t << flush << std::endl;
  wflow::asio::io_service ios;
  wflow::workflow_options wo;
  wo.id = "overflow_reset";
  wo.maxsize=100;
  wo.overflow_reset = true;
  wflow::workflow flw(ios, wo);
  flw.start();
  size_t counter = 0;
  size_t lost_counter = 0;
  for (int i = 0; i != 142; i++)
  {
    flw.post([&](){++counter;}, [&](){++lost_counter;});
    if ( i == 109 )
    {
      ios.run();
    }
  }
  ios.reset();
  ios.run();
  t << equal<expect, size_t>(counter, 32) << FAS_FL;
  t << equal<expect, size_t>(lost_counter, 110) << FAS_FL;
}

UNIT(shutdown, "")
{
  using namespace ::fas::testing;
  wflow::workflow_options wo;
  wo.id = "shutdown";
  wo.threads = 4;
  std::mutex mutex;
  std::set<std::thread::id> threads_ids;
  size_t count = 0;
  wflow::workflow flw(wo);
  for (size_t i = 0; i < 16; ++i)
  {
    flw.post([&](){
      std::this_thread::sleep_for( std::chrono::milliseconds(10)  );
      std::lock_guard<std::mutex> lk(mutex);
      t << message("Thread ID: ") << std::this_thread::get_id() << " count=" << count;
      t << flush;
      ++count;
      threads_ids.insert(std::this_thread::get_id());
    });
  }
  flw.create_timer(std::chrono::milliseconds(10), [&]()
  {
    std::lock_guard<std::mutex> lk(mutex); 
    t << message("timer"); 
    t << flush;
    return true;
  }, wflow::expires_at::before );
  
  flw.safe_post(std::chrono::milliseconds(100), [&](){
    std::lock_guard<std::mutex> lk(mutex); 
    t << message("delayed"); 
    t << flush;
    return true;
  });
  { std::lock_guard<std::mutex> lk(mutex); t << message("start..."); }
  flw.start();
  { std::lock_guard<std::mutex> lk(mutex); t << message("shutdown..."); }
  flw.shutdown();
  { std::lock_guard<std::mutex> lk(mutex); t << message("wait..."); }
  flw.wait();
  { std::lock_guard<std::mutex> lk(mutex); t << message("done!"); t << flush; }
  t << equal<expect, size_t>(count, 16) << FAS_FL;
  t << equal<expect, size_t>(threads_ids.size(), 4) << FAS_FL;
  
  t << message("=====================================");
  for (size_t i = 0; i < 16; ++i)
  {
    flw.post([&](){++count;});
  }
  t << message("=====================================");  
  flw.start();
  t << message("=====================================");
  flw.shutdown();
  flw.wait();
  t << equal<expect, size_t>(count, 32) << FAS_FL;
}

}


BEGIN_SUITE(workflow, "")
  ADD_UNIT(workflow1)
  ADD_UNIT(workflow2)
  ADD_UNIT(workflow3)
  ADD_UNIT(rate_limit)
  ADD_UNIT(requester1)
  ADD_UNIT(overflow_reset)
  ADD_UNIT(shutdown)
END_SUITE(workflow)

