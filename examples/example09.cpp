#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
 * @example example09.cpp
 * @brief Тестирование пропускной способности wflow::workflow с различным числом потоков на ограниченной по размеру очереди.
 * @details Число потоков передается параметром командной строки. Как видно из результатов, для легковесных заданий, наиболее 
 * оптимальным является конфигурация с одним выделенным потоком или однопоточный вариант. Чтобы избежать потерь достаточно 
 * увеличить размер очереди.
 * @remark Для отложенного на десять секунд задания для завершения работы не использовали отдельный wflow::workflow т.к. очередь
 * состоит исключительно из легковесных заданий и не бывает такого, что все потоки зависают на обработке без возможности отработать 
 * таймер. Смотри /ref example11.cpp, где показана подобная задержка.
 */

/*
| threads | counter  |  rate      | dropped | 
| ------- | -------- | ---------- | ------- |
|    0    | 21 млн   | ~2 млн/с   | ~50тыс  |
|    1    | 30 млн   | ~3 млн/с   | ~10тыс  |
|    2    | 7 млн    | ~710 тыс/с | ~4тыс   |
|    3    | 5.7 млн  | ~573 тыс/с | ~11тыс  |
|    4    | 4.6 млн  | ~467 тыс/с | ~3тыс   |
|    5    | 4.2 млн  | ~427 тыс/с | 0       |
|    6    | 3.7 млн  | ~370 тыс/с | 0       |
|    7    | 3.5 млн  | ~358 тыс/с | 0       |
|    8    | 3.3 млн  | ~331 тыс/с | 0       |
|    9    | 3.2 млн  | ~329 тыс/с | 0       |
|    10   | 3.3 млн  | ~333 тыс/с | 0       |
|    15   | 3.6 млн  | ~367 тыс/с | 0       |
|    20   | 3.8 млн  | ~382 тыс/с | 0       |
|    30   | 3.1 млн  | ~310 тыс/с | 0       |
|    50   | 2.1 млн  | ~216 тыс/с | 0       |
|    100  | 1   млн  | ~108 тыс/с | 0       | 
|    200  | 565 тыс  | ~56  тыс/с | 0       |
|    300  | 394 тыс  | ~39  тыс/с | 0       |
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
