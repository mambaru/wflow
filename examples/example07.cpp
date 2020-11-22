#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>
#include <mutex>

/**
 * @example example07.cpp
 * @brief Пример многопоточной обработки с ограничением скорости обработки.
 * @details В этом примере ограничение на десять заданий в секунду. Первые десять заданий выполняются моментально,
 * а остальные более менее равномерно, но так, чтобы не превышать заданную скорость.
 */
int main()
{
  boost::asio::io_context ios;
  wflow::workflow_options opt;
  opt.threads = 5;
  opt.rate_limit = 10;
  wflow::workflow wf(ios, opt);
  wf.start();
  std::mutex m;
  for (int i = 0; i < 100; ++i)
  {
    wf.post( [&, i](){
      std::lock_guard<std::mutex> lk(m);
      std::cout << i << " " << std::this_thread::get_id() << std::endl;
    });
  }
  wf.shutdown();
  wf.wait();
}
