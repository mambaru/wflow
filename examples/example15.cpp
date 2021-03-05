#include <wflow/workflow.hpp>
#include <wflow/load.hpp>
#include <chrono>
#include <iostream>

/**
 * @example example15.cpp
 * @brief Пример использования JSON-конфигурации
 * @details При запуске без параметров выводится сгенерированный JSON со значениями по умолчанию. В качестве параметра
 * передается JSON-конфигурация для wflow::workflow, через который данные из STDIN будут отправлены в STDOUT. Можно поэкспериментировать
 * с многопоточностью, задержками и ограничениями скорости обработки через конфиг.
 * ```bash
 * time cat ../examples/example15.cpp | ./examples/example15 ../examples/example15-1.json
 * ```
 */

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << wflow::dump( wflow::workflow_options() ) << std::endl;
    return 0;
  }

  wflow::workflow_options opt;
  std::string err;
  if ( !wflow::load(argv[1], &opt, &err) )
  {
    std::cerr << "ERROR: " << err << std::endl;
    return -1;
  }
  boost::asio::io_context ios;
  wflow::workflow wf(ios, opt);
  wf.start();

  std::atomic<size_t> count(0);

  while( std::cin.good() )
  {
    std::string str;
    std::getline (std::cin,str);
    ++count;
    wf.post([str, &count](){
      std::cout << str << std::endl;
      --count;
    });
  }
  wf.create_timer(std::chrono::milliseconds(100), [&count, &ios](){
    if (count!=0)
      return true;
    ios.stop();
    return false;
  });
  std::cout << "Read DONE!" << std::endl;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> wrk(ios.get_executor());
  ios.run();
  std::cout << "BYE!" << std::endl;
}
