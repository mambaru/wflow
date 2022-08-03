#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <fas/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
 * @example example08.cpp
 * @brief Пример динамической реконфигурации.
 * @details В этом примере ограничение по таймеру два раза реконфигурируем wflow::workflow, одновременно с этим, с отдельного потока,
 * максимально загружаем его пустыми заданиями.
 */
int main()
{
  std::cout << "main thread " << std::this_thread::get_id() << std::endl;
  volatile int time_counter = 0;
  boost::asio::io_context ios;
  wflow::workflow_options opt;
  opt.threads = 3;
  wflow::workflow wf(ios, opt);
  wf.start();
  size_t post_counter =0;
  std::thread([&time_counter, &wf, &post_counter](){
    for (;time_counter < 60; ++post_counter)
      wf.post([]() noexcept{});
  }).detach();
  wf.create_timer(
    std::chrono::milliseconds(100),
    [&time_counter, &wf, &opt, &ios](){
      std::cout << time_counter << " " << std::this_thread::get_id() << std::endl;
      time_counter = time_counter + 1;
      if ( time_counter == 20 )
      {
        opt.threads = 0;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl;
      }
      else if ( time_counter == 40 )
      {
        opt.threads = 6;
        wf.reconfigure(opt);
        std::cout << "---------------------" << std::endl;
      }
      else if ( time_counter == 60)
      {
        std::cout << "---------------------" << std::endl;
        ios.stop();
      }

      return time_counter < 60;
    });

  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> wrk(ios.get_executor());
  ios.run();

  std::cout << post_counter << std::endl;
}
