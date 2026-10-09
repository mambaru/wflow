#include <wflow/workflow.hpp>
#include <wflow/owner.hpp>
#include <iostream>
#include <chrono>

/**
 * @example example19.cpp
 * @brief Owner::tracking: отмена заданий при «закрытии» соединения (io_id).
 * @details Включаем per-io_id токены. Оба задания для клиента 42 обёрнуты через tracking
 * заранее. После release_tracking(42) ещё не выполненное задание вызывает alt —
 * как будто клиент закрыл соединение.
 */

/* Output:
  conn 42: request handled
  conn 42 closed
  conn 42: cancelled (client gone)
*/

int main()
{
  boost::asio::io_context ios;
  wflow::workflow wf(ios);
  wflow::owner own;
  own.enable_tracking(true);

  const wflow::owner::io_id_t conn_id = 42;

  // Оба handler'а берут один и тот же tracking-токен (пока map содержит io_id).
  wf.safe_post(std::chrono::milliseconds(50), own.tracking(
    conn_id,
    [](){ std::cout << "conn 42: request handled" << std::endl; },
    [](){ std::cout << "conn 42: cancelled early" << std::endl; }
  ));

  wf.safe_post(std::chrono::milliseconds(150), own.tracking(
    conn_id,
    [](){ std::cout << "conn 42: late work" << std::endl; },
    [](){ std::cout << "conn 42: cancelled (client gone)" << std::endl; }
  ));

  wf.safe_post(std::chrono::milliseconds(100), [&own]()
  {
    own.release_tracking(conn_id);
    std::cout << "conn 42 closed" << std::endl;
  });

  wf.safe_post(std::chrono::milliseconds(200), [&ios]()
  {
    ios.stop();
  });

  ios.run();
}
