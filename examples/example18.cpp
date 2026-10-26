#include <wflow/workflow.hpp>
#include <wflow/owner.hpp>
#include <iostream>
#include <chrono>
#include <memory>

/**
 * @example example18.cpp
 * @brief Owner::wrap и reset: отложенное задание не трогает уже остановленный объект.
 * @details Ставим два отложенных вызова через wrap. После первого срабатывания вызываем
 * reset — второй уходит в альтернативный handler и не читает поля сервиса.
 */

/* Output:
  tick #1 value=1
  service stopped
  skipped (owner reset), value was 1
*/

namespace {

struct service
{
  wflow::owner own;
  int value = 0;

  void schedule(wflow::workflow& wf, std::chrono::milliseconds delay)
  {
    wf.safe_post(delay, own.wrap(
      [this]()
      {
        ++value;
        std::cout << "tick #" << value << " value=" << value << std::endl;
      },
      [this]()
      {
        std::cout << "skipped (owner reset), value was " << value << std::endl;
      }
    ));
  }

  void stop()
  {
    own.reset();
    std::cout << "service stopped" << std::endl;
  }
};

}

int main()
{
  boost::asio::io_context ios;
  wflow::workflow wf(ios);
  auto svc = std::make_shared<service>();

  svc->schedule(wf, std::chrono::milliseconds(100));
  svc->schedule(wf, std::chrono::milliseconds(300));

  wf.safe_post(std::chrono::milliseconds(200), [svc]()
  {
    svc->stop();
  });

  wf.safe_post(std::chrono::milliseconds(400), [&ios]()
  {
    ios.stop();
  });

  ios.run();
}
