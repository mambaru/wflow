#include <fas/testing.hpp>
#include <wflow/task/task_manager.hpp>
#include <wflow/workflow.hpp>
#include <wflow/system/memory.hpp>
#include <chrono>

UNIT(workflow1, "")
{
  using namespace ::fas::testing;
  t << flush;
  wflow::asio::io_service io;
  wflow::workflow_options opt;
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
  
  queue.timer()->create(std::chrono::milliseconds(400), [&t, &counter, &m](){
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
  t << equal< assert,int >( counter, 7 ) << FAS_FL ;
  
}

UNIT(workflow2, "5 сообщений, одно 'теряется' и одно остаеться в очереди")
{
  using namespace ::fas::testing;
  ::wflow::asio::io_service io;  
  ::wflow::workflow_options opt;
  std::atomic<int> counter(0);
  std::atomic<int> dropped(0);
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
  std::atomic<int> dropped(0);
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
      std::chrono::milliseconds(200*(i+1) ), 
      [&t, i, &counter, &io]()
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
  void method( request::ptr, response::handler h) { h( std::unique_ptr<response>( new response() ) ); };
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
    /*f, &foo::method,*/
    [f](foo::request::ptr req, foo::response::handler callback)
    {
      f->method(std::move(req), callback);
      return true;
    },
    [&id, &ios, &finish]( foo::response::ptr ) -> foo::request::ptr
    {
      std::cout << std::endl << id << std::endl;
      finish = high_resolution_clock::now();
      ios.stop();
      return std::make_unique<foo::request>();
    }
  );
  t << message("requester: ") << id;
  flw.start();
  ios.run();
  auto interval = duration_cast<milliseconds>(finish - start).count();
  t << equal<expect>(interval, 0) << FAS_FL;

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
  flw.start();

  auto start = high_resolution_clock::now();
  auto finish = start;
  
  size_t counter = 0;
  for (int i = 0; i != 200; i++)
    flw.post([&](){++counter;});
  ios.run();
  finish = high_resolution_clock::now();
  t << equal<expect, size_t>(counter, 200) << FAS_FL;
  t << equal<expect, size_t>(2000, duration_cast<milliseconds>(finish - start).count()) << FAS_FL;
}




BEGIN_SUITE(workflow, "")
  ADD_UNIT(workflow1)
  ADD_UNIT(workflow2)
  ADD_UNIT(workflow3)
  ADD_UNIT(rate_limit)
  ADD_UNIT(requester1)
END_SUITE(workflow)

