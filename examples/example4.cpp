#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>

/** Output:
  */

int main()
{
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 0;
  opt.rate_limit = 10;
  wflow::workflow wf(ios, opt);
  wf.start();
  int counter = 100;
  
  for (int i = 0; i < 100; ++i)
  {
    wf.post( [&ios, i, &counter](){
      std::cout << i << std::endl;
      if ( --counter == 0 )
        ios.stop();
    });
  }
  
  boost::asio::io_service::work wrk(ios);
  ios.run();
  wf.stop();
}
