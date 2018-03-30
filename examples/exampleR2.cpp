#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
| threads | counter  |  rate   | dropped | 
| ------- | -------- | ------- | ------- |
|    0    | 21 млн   | 2 млн/с | ~50тыс  |
|    1    | 21 млн   | 2 млн/с | ~50тыс  | 

*/
int main(int argc, char* argv[])
{
  size_t threads = 0;
  if ( argc > 1 )
    threads = size_t(std::atol(argv[1]));
  
  wflow::asio::io_service ios;
   wflow::asio::io_service::work wrk(ios);
  wflow::workflow_options opt;
  opt.threads = threads;
  opt.maxsize = 128;
  opt.wrnsize = 128;
  opt.control_ms = 1000;
  wflow::workflow wf(ios, opt);
  wf.start();
  std::atomic<bool> run(true);
  size_t counter = 0;
  
  std::thread t([&run, &wf, &counter](){
    for (;run; ++counter)
      wf.post([](){});
  });
  
  wf.safe_post( std::chrono::seconds(10), [&ios, &run](){ ios.stop(); run = false;});
  ios.run();
  t.join();
  std::cout << "dropped " << wf.dropped() << std::endl;
  std::cout << counter << std::endl;
}
