#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <iostream>
#include <chrono>


int main()
{
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.threads = 5;
  wflow::workflow wf(ios, opt);
  wf.start();
  volatile int counter = 100;
  std::mutex m;
  auto show = [&counter, &m](int value) 
  {
    usleep( static_cast<useconds_t>(std::rand() % 1000) );
    std::lock_guard<std::mutex> lk(m);
    --counter;
    std::cout << "post " << value << " thread_id: " << std::this_thread::get_id() << std::endl; 
  };
  for (int i = 0; i < 100; ++i)
    wf.post( std::bind(show, i));
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
