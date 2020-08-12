#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example04.cpp
 * @brief Пример ограничения размера очереди для отложенных.
 * @details Для отложенных заданий, сначала создается таймер на постановку в очередь. Значение таймера номинальное, но сработает он только
 * после запуска io_context::run(), поэтому в отличии от \ref example03.cpp переполнение произойдет уже после запуска.
 */

/**
 * Output:
    Run!
    drop 5
    drop 6
    drop 7
    drop 8
    drop 9
    post 0
    post 1
    post 2
    post 3
    post 4
*/
int main()
{
  boost::asio::io_context ios;
  wflow::workflow_options opt;
  opt.maxsize = 5;
  wflow::workflow wf(ios, opt);

  for (int i = 0; i < 10; ++i)
  {
    wf.post(
      std::chrono::nanoseconds(1),
      [i](){ std::cout << "post " << i << std::endl; },
      [i](){ std::cout << "drop " << i << std::endl; }
    );
  }

  std::cout << "Run!" << std::endl;
  // Ожидаем выполнение всех заданий
  ios.run();
}
