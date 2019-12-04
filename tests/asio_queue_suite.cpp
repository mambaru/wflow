#include <fas/testing.hpp>
#include <wflow/queue/asio_queue.hpp>
#include "delayed_common_suite.hpp"

UNIT(asio_queue1, "")
{
  ::wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit1(t, *pq);
}


UNIT(asio_queue2, "")
{
  ::wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit2(t, *pq);
}

UNIT(asio_queue3, "")
{
  ::wflow::asio::io_service io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit3(t, *pq);
}

BEGIN_SUITE(asio_queue, "")
  ADD_UNIT(asio_queue1)
  ADD_UNIT(asio_queue2)
  ADD_UNIT(asio_queue3)
END_SUITE(asio_queue)

