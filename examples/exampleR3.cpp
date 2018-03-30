#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
| threads | counter  |  rate      | dropped | 
| ------- | -------- | ---------- | ------- |
|    0    | 21 млн   | ~2 млн/с   |    0    |
|    1    | 29 млн   | ~2.9 млн/с |    0    |
|    2    | 6.1 млн  | ~615 тыс/с |    0    |
|    3    | 5.4 млн  | ~542 тыс/с |    0    |
|    4    | 4.9 млн  | ~493 тыс/с |    0    |
|    5    | 4.2 млн  | ~423 тыс/с |    0    |
|    6    | 3.6 млн  | ~368 тыс/с |    0    |
|    7    | 3.5 млн  | ~353 тыс/с |    0    |
|    8    | 3.2 млн  | ~323 тыс/с |    0    |
|    9    | 3.3 млн  | ~337 тыс/с |    0    |
|    10   | 3.3 млн  | ~338 тыс/с |    0    |
|    15   | 3.5 млн  | ~357 тыс/с |    0    |
|    20   | 3.7 млн  | ~368 тыс/с |    0    |
|    30   | 3.1 млн  | ~312 тыс/с |    0    |
|    50   | 2.2 млн  | ~219 тыс/с |    0    |
|    100  | 1.4 млн  | ~148 тыс/с |    0    |
|    200  | 677 тыс  | ~67  тыс/с |    0    |
|    300  | 379 тыс  | ~38  тыс/с |    0    |
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
  wflow::workflow wf(ios, opt);
  wf.start();
  std::atomic<bool> run(true);
  size_t counter = 0;
  
  std::thread t([&run, &wf, &counter](){
    for (;run; ++counter)
      wf.safe_post([](){});
  });
  
  wf.safe_post( std::chrono::seconds(10), [&ios, &run](){ ios.stop(); run = false;});
  ios.run();
  t.join();
  std::cout << "dropped " << wf.dropped() << std::endl;
  std::cout << counter << std::endl;
}
