#include <memory>
#include <fas/testing.hpp>
#include <wflow/timer/timer_manager.hpp>
#include <wflow/queue/delayed_queue.hpp>
#include <wflow/queue/asio_queue.hpp>
#include <wflow/system/memory.hpp>


struct test_request
{
  typedef std::unique_ptr<test_request> ptr;
};

struct test_response
{
  typedef std::unique_ptr<test_response> ptr;
  typedef std::function<void(ptr)> handler;
};

class test_class
{
public:
  void method( test_request::ptr, test_response::handler handler)
  {
    handler( std::unique_ptr<test_response>(new test_response() ) );
  }
};


UNIT(timer_manager1, "")
{
  using namespace ::fas::testing;
  wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  auto tm = std::make_shared< ::wflow::timer_manager< ::wflow::asio_queue > >( pq );
  std::atomic<int> counter(0);
  tm->create( std::chrono::milliseconds(200), [&counter, &t]()
  {
    ++counter;
    t << message("tick1");
    return true;
  } );
  tm->create(std::chrono::milliseconds(400), [&counter, &t]()
  {
    ++counter;
    t << message("tick2");
    return true;
  } );
  
  auto tc = std::make_shared<test_class>();
  tm->create<test_request, test_response>( 
    "", std::chrono::milliseconds(600), 
    [tc](test_request::ptr req, test_response::handler callback)
    {
      tc->method(std::move(req), callback);
      return true;
    },
    [&t, &counter]( test_response::ptr) -> test_request::ptr 
    {
      ++counter;
      t << message("call1");
      return test_request::ptr( new test_request() );
    }
  );
  
  for ( int i =0 ; i < 4; i++)
    io.run_one();
  
  t << equal< assert,int >( counter, 4 ) << "counter 4!=" << counter ;
}

UNIT(timer_manager2, "")
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  auto tm = std::make_shared< ::wflow::timer_manager< ::wflow::asio_queue > >( pq );

  auto start = high_resolution_clock::now();
  auto finish = start;
  tm->create( std::chrono::milliseconds(0), std::chrono::milliseconds(200), [&finish, &t]()
  {
    finish = high_resolution_clock::now();
    t << message("tick1");
    return false;
  } );
  io.run();
  auto interval = duration_cast<milliseconds>(finish - start).count();
  t << equal<expect>(interval, 0) << FAS_FL;
  t << nothing;
}

UNIT(timer_manager3, "")
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  auto tm = std::make_shared< ::wflow::timer_manager< ::wflow::asio_queue > >( pq );

  auto start = high_resolution_clock::now();
  auto finish = start;
  tm->create( "", std::chrono::milliseconds(200), [&finish, &t]()
  {
    finish = high_resolution_clock::now();
    t << message("tick1");
    return false;
  }, wflow::expires_at::before );
  io.run();
  auto interval = duration_cast<milliseconds>(finish - start).count();
  t << equal<expect>(interval, 0) << FAS_FL;
  t << nothing;
}

BEGIN_SUITE(timer_manager, "")
  ADD_UNIT(timer_manager1)
  ADD_UNIT(timer_manager2)
  ADD_UNIT(timer_manager3)
END_SUITE(timer_manager)

