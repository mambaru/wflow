#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @brief Простые примеры защищенных заданий в однопоточном режиме
 * @remark Ожидание выполнения всех заданий с помощью io_service::run работает только в однопоточном режиме
 */
int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  
  // Простое задание 
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );
  
  // Отложенное задание 
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );
  
  // Задание на конкретный момент времени 
  auto tp = std::chrono::system_clock::now();
  tp += std::chrono::seconds(2);
  wf.safe_post( tp, [](){ std::cout << "Safe post in time point" << std::endl; } );
  
  // Ожидаем выполнение всех заданий 
  ios.run();
}
