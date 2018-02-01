#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>


/** Output:
  * first message
  * timer count=4
  * after 1 seconds
  * timer count=3
  * after 2 seconds
  * timer count=2
  * after 3 seconds
  * timer count=1
  * after 4 seconds
  */

int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  
  wf.create_timer( 
    std::chrono::milliseconds(500),
    std::chrono::milliseconds(1000),
    []()->bool
    {
      static int count = 4;
      std::cout << "timer count=" << count << std::endl;
      return --count != 0;
    }
  );
  
  wf.post( std::chrono::nanoseconds(1000000000),[](){ std::cout << "after 1 seconds " << std::endl; }, nullptr);
  wf.post( std::chrono::milliseconds(2000),     [](){ std::cout << "after 2 seconds " << std::endl; }, nullptr);
  wf.post( std::chrono::seconds(3),             [](){ std::cout << "after 3 seconds " << std::endl; }, nullptr);
  wf.post( std::chrono::microseconds(4000000),  [](){ std::cout << "after 4 seconds " << std::endl; }, nullptr);
  wf.post( [](){ std::cout << "first message " << std::endl; }, nullptr);
  ios.run();
}
