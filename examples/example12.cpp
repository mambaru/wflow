#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example12.cpp
 * @brief Пример того, как решить проблему с задержкой таймкра из примера /ref example11.cpp
 * @details Для этого используем отдельный wflow::workflow для таймеров
 */


/**
   It was expected 2 seconds, but waited 2 seconds
   real    0m4.004s
   user    0m0.000s
   sys     0m0.004s
 */

int main()
{
  wflow::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 10;
  wflow::workflow wf(ios, opt);
  opt.threads = 0;
  wflow::workflow wf2(ios, opt);
  wf.start();

  for (int i=0; i < 10; ++i)
    wf.post([](){ sleep(4);});
  
  
  time_t beg = time(0);
  wf2.safe_post( 
    std::chrono::seconds(2), 
    [&ios, beg]()
    {
      time_t now = time(0);
      std::cout << "It was expected 2 seconds, but waited "<< now - beg << " seconds" << std::endl;
      ios.stop();
    });
  wflow::asio::io_service::work wrk(ios);
  ios.run();
  
}
