#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example03.cpp
 * @brief Пример ограничения размера очереди.
 * @details В этом примере при превышении размера, задания не ставятся в очередь, а сразу вызывается drop-обработчик до запуска io_service::run().
 */

/**
 * Output:
    drop 5
    drop 6
    drop 7
    drop 8
    drop 9
    Run!
    post 0
    post 1
    post 2
    post 3
    post 4
*/
int main()
{
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.maxsize = 5;
  wflow::workflow wf(ios, opt);
  
  for (int i = 0; i < 10; ++i)
  {
    wf.post( 
      [i](){ std::cout << "post " << i << std::endl; },
      [i](){ std::cout << "drop " << i << std::endl; } 
    );
  }
  
  std::cout << "Run!" << std::endl;
  // Ожидаем выполнение всех заданий 
  ios.run();
}
