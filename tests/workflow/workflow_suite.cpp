#include <fas/testing.hpp>
#include <iow/workflow/task_manager.hpp>
#include <iow/workflow/bique.hpp>
#include <iow/workflow/workflow.hpp>
#include <iow/logger/global_log.hpp>
#include <chrono>

UNIT(workflow1, "")
{
  using namespace ::fas::testing;
  
  ::iow::asio::io_service io;

  ::iow::task_manager queue(io, 0, 3, false);
  queue.start();
  std::atomic<int> counter(0);
  queue.post([&t, &counter](){
    ++counter;
    t << message("post");
  }, nullptr);
  queue.timer()->create(std::chrono::milliseconds(400), [&t, &counter](){
    ++counter;
    t << message("timer 400ms");
    return true;
  });
  queue.delayed_post( std::chrono::milliseconds(600), [&t, &counter](){
    ++counter;
    t << message("delayed post 600ms");
  }, nullptr);
  for (int i =0 ; i < 3 ; ++i)
  {
    queue.delayed_post( std::chrono::milliseconds(300 + i*300), [&t, &counter, i](){
      ++counter;
      t << message("delayed post ") << 300 + i*300 << "ms";
    }, nullptr);
  }
  sleep(1);
  std::cout << "stop..." << std::endl;
  queue.stop();
  std::cout << "stop!" << std::endl;
  t << equal< assert,int >( counter, 7 ) << "counter 7!=" << counter ;
    std::cout << "stop!2" << std::endl;

  
}

UNIT(workflow2, "5 сообщений, одно 'теряется' и одно остаеться в очереди")
{
  using namespace ::fas::testing;
  ::iow::asio::io_service io;  
  ::iow::asio::io_service::work wrk(io);
  ::iow::workflow_options opt;
  ::iow::workflow* pw;
  
  bool ready = false;
  std::atomic<int> counter(0);
  std::atomic<int> dropped(0);
  opt.maxsize = 5; // + handler timer
  opt.use_io_service = true;
  opt.threads = 1;
  opt.control_ms = 1000;
  opt.id = "test";
  opt.control_handler = [&pw, &ready, &counter, &io]()->bool 
  {
    if ( ready )
      return false;
    if ( counter < 3 ) 
      return true;
    ready = true;
    /*
    if ( ready )
      return false;
    //ready = pw->queue_size() == 1;
    if ( !ready )
      return true;
    */
    
    io.stop();
    return false;
  };
  ::iow::workflow wfl(io, opt);
  pw = &wfl;
  wfl.start();
  wfl.manager(); // for cppcheck
  
  for (int i =0 ; i < 5; i++)
    wfl.post( std::chrono::milliseconds(i*1000 + 1000),  [&t, i, &counter](){
      t << message("for 0..5 i=") << i;
      t << flush;
      ++counter;
    }, [&dropped](){     
      ++dropped;
    } );
  
  io.run();
  /*while ( counter < 5)
    io.run_one();
    */
  t << is_true<expect>(ready) << FAS_TESTING_FILE_LINE;
  t << is_true<expect>(pw->dropped()==1) << FAS_TESTING_FILE_LINE;
  t << is_true<expect>(dropped==1) << FAS_TESTING_FILE_LINE;
  t << is_true<expect>( pw->timer_count()==1 ) << FAS_TESTING_FILE_LINE;
  t << is_true<expect>( pw->queue_size()==1  ) << pw->queue_size() << " " << FAS_TESTING_FILE_LINE;
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
  void method( request::ptr, response::handler h) { h( std::make_unique<response>() ); };
};

UNIT(requester1, "")
{
  using namespace ::fas::testing;
  using namespace std::chrono;
  // log for cppcheck 
  iow::init_log(iow::log_writer() );
  iow::log_status();
  auto f = std::make_shared<foo>();
  iow::asio::io_service ios;
  iow::workflow_options wo;
  wo.threads = 0;
  iow::workflow flw(ios, wo);
  iow::workflow::timer_id_t id;
  auto start = high_resolution_clock::now();
  auto finish = start;
  id = flw.create_requester< foo::request, foo::response >(
    milliseconds(0), milliseconds(1000), f, &foo::method,
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

BEGIN_SUITE(workflow, "")
  ADD_UNIT(workflow1)
  ADD_UNIT(workflow2)
  ADD_UNIT(requester1)
END_SUITE(workflow)

