#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>
#include <mutex>

/**
 * @example example07.cpp
 * @brief Пример многопоточной обработки с ограничением скорости обработки 
 * @details В этом примере ограничение на 10 заданий в секунду. Прервые 10 заданий выполняются моментально, 
 * а остальные более менее равномерно, но так, чтобы не превышать заданную скорость. 
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
  
  /* Используем boost::asio::io_service::work чтобы не "вылетать" т.к. по факту ios не используется в качестве очереди */
  boost::asio::io_service::work wrk(ios);
  ios.run();
}
