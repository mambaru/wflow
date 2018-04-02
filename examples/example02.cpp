#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example02.cpp
 * @brief Простые примеры незащищенных заданий в однопоточном режиме
 * @remark Ожидание выполнения всех заданий с помощью io_service::run работает только в однопоточном режиме
 */
int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  
  // Простое задание 
  wf.post( [](){ std::cout << "Simple unsafe post  " << std::endl; } );
  
  // Отложенное задание 
  wf.post( std::chrono::seconds(4), [](){ std::cout << "Safe unpost after delay 4 second " << std::endl; } );
  
  // Задание на конкретный момент времени 
  auto tp = std::chrono::system_clock::now();
  tp += std::chrono::seconds(2);
  wf.post( tp, [](){ std::cout << "Safe unpost in time point" << std::endl; } );
  
  // Ожидаем выполнение всех заданий 
  ios.run();
}
