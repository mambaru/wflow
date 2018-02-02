#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>

/** Output:
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
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.maxsize = 5;
  wflow::workflow wf(ios, opt);

  auto show = [](int value) { std::cout << "post " << value << std::endl; };
  auto drop = [](int value) { std::cout << "drop " << value << std::endl; };
  for (int i = 0; i < 10; ++i)
    wf.post( std::bind(show, i), std::bind(drop, i) );
  ios.run();
}
