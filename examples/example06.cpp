#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example06.cpp
 * @brief Пример многопоточной обработки
 * @details В этом примере, в обработчике немного засыпаем, блокируя поток, чтобы подключились другие потоки и получить красивый вывод.
 */

void task(int value);

volatile int counter = 100;
std::mutex m;

void task(int value) 
{
  usleep( static_cast<useconds_t>(std::rand() % 1000) );
  std::lock_guard<std::mutex> lk(m);
  --counter;
  std::cout << "post " << value << " thread_id: " << std::this_thread::get_id() << std::endl; 
}

int main()
{
  wflow::workflow_options opt;
  opt.threads = 5;
  wflow::workflow wf(opt);
  wf.start();
  for (int i = 0; i < 100; ++i)
    wf.post( std::bind(task, i));
  for(;;)
  {
    {
      std::lock_guard<std::mutex> lk(m);
      if (counter==0)
        break;
      std::cout << "wait... counter=" << counter << std::endl;
    }
    usleep(1000);
  }
}
