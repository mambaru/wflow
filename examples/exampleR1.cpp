#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

int main()
{
  std::cout << "main thread " << std::this_thread::get_id() << std::endl; 
  int count = 0;
  wflow::asio::io_service ios;
   wflow::asio::io_service::work wrk(ios);
  wflow::workflow_options opt;
  opt.threads = 3;
  wflow::workflow wf(ios, opt);
  wf.start();
  size_t trash =0;
  std::thread([&count, &wf, &trash](){
    for (;count < 60; ++trash)
      wf.post([](){});
  }).detach();
  wf.create_timer(
    std::chrono::milliseconds(100), 
    [&count, &wf, &opt, &ios](){ 
      std::cout << count << " " << std::this_thread::get_id() << std::endl; 
      ++count;
      if ( count == 20 )
      {
        opt.threads = 0;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl; 
      }
      else if ( count == 40 )
      {
        opt.threads = 6;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl; 
      }
      else if ( count == 60)
      {
        std::cout << "---------------------" << std::endl; 
        ios.stop();
      }
      
      return count < 60;
    });
  ios.run();
  std::cout << trash << std::endl;
}
