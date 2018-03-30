#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>
#include <mutex>

/** Output:
  */

int main()
{
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 5;
  opt.rate_limit = 10;
  wflow::workflow wf(ios, opt);
  wf.start();
  int counter = 1000;
  std::mutex m;
  for (int i = 0; i < 1000; ++i)
  {
    wf.post( [&, i](){
      std::lock_guard<std::mutex> lk(m);
      std::cout << i << " counter=" << counter << std::endl;
      
      if ( --counter == 0 )
        ios.stop();
    });
  }
  
  boost::asio::io_service::work wrk(ios);
  ios.run();
}
