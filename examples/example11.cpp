#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example11.cpp
 * @brief Пример того, как можно забить очередь так, чтобы таймеры не сработали вовремя.
 * @details В конфигурации 4 потока отправляем 4 задания которые засыпают и не отпускают 
 * поток на 4 секунды. Тут же отправляем отложенное на 2 секунды задание, но оно сработает
 * только через 4 секунды, когда освободится один из потоков.
 */

/* > time example11
  It was expected 2 seconds, but waited 4 seconds
  real    0m4.005s
  user    0m0.000s
  sys     0m0.006s
 */

int main()
{
  wflow::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 4;
  wflow::workflow wf(ios, opt);
  wf.start();

  for (int i=0; i < 4; ++i)
    wf.post([](){ sleep(4);});
  
  time_t beg = time(0);
  wf.safe_post( 
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
