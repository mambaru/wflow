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
  int time_counter = 0;
  wflow::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 3;
  wflow::workflow wf(ios, opt);
  wf.start();
  size_t post_counter =0;
  std::thread([&time_counter, &wf, &post_counter](){
    for (;time_counter < 60; ++post_counter)
      wf.post([](){});
  }).detach();
  wf.create_timer(
    std::chrono::milliseconds(100), 
    [&time_counter, &wf, &opt, &ios](){ 
      std::cout << time_counter << " " << std::this_thread::get_id() << std::endl; 
      ++time_counter;
      if ( time_counter == 20 )
      {
        opt.threads = 0;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl; 
      }
      else if ( time_counter == 40 )
      {
        opt.threads = 6;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl; 
      }
      else if ( time_counter == 60)
      {
        std::cout << "---------------------" << std::endl; 
        ios.stop();
      }
      
      return time_counter < 60;
    });
  
  wflow::asio::io_service::work wrk(ios);
  ios.run();
  
  std::cout << post_counter << std::endl;
}
