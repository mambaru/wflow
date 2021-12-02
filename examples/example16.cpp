#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example05.cpp
 * @brief Еще пример таймера и различных заданий.
 */

/** Output:
  * first message
  * timer count=4
  * after 1 seconds
  * timer count=3
  * after 2 seconds
  * timer count=2
  * after 3 seconds
  * timer count=1
  * after 4 seconds
  */

int main()
{
  boost::asio::io_context ios;
  wflow::workflow wf(ios);

  time_t now=time(nullptr);
  int count = 4;
  wf.create_timer(
    "1m20s",
    std::chrono::milliseconds(100),
    [&count]()->bool
    {
      std::cout << "timer count=" << count << std::endl;
      return --count != 0;
    }
  );

  wf.create_timer(
    std::chrono::seconds(1),
    [now,&count]()->bool
    {
      std::cout << std::time(nullptr) - now << std::endl;
      return count != 0;
    }
  );
  
  wf.post( "*/10 * * * * *", []() { time_t v = time(nullptr) /*- now*/ ; std::cout << "cron 10 sec " <<  v << std::endl;} );

  ios.run();
}
