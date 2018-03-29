#include <wflow/workflow.hpp>
#include <iostream>
#include <chrono>

/* Output:
      Run!
      job №5 is drop
      job №6 is drop
      job №7 is drop
      job №8 is drop
      job №9 is drop
      job №0 is done
      job №1 is done
      job №2 is done
      job №3 is done
      job №4 is done
*/
int main()
{
  boost::asio::io_service ios;
  wflow::workflow_options opt;
  opt.maxsize = 5;
  wflow::workflow wf(ios, opt);
  
  for (int i = 0; i < 10; ++i)
  {
    wf.post( 
      std::chrono::nanoseconds(1), 
      [i](){ std::cout << "job №" << i << " is done" << std::endl; },
      [i](){ std::cout << "job №" << i << " is drop" << std::endl; } 
    );
  }
  
  std::cout << "Run!" << std::endl;
  // Ожидаем выполнение всех заданий 
  ios.run();
}
