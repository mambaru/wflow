#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>


int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  
  // Налогичен 
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );
  
  auto now = std::chrono::system_clock::now();
  now += std::chrono::seconds(2);
  wf.safe_post( now, [](){ std::cout << "Safe post in time point" << std::endl; } );
  
  if ( !wf.post([](){ std::cout << "Simple unsafe post" << std::endl;}) ) 
    std::cout << "Drop  handler if queue overflow" << std::endl; 
  
  ios.run();
}
