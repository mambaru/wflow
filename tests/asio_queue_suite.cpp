#include <fas/testing.hpp>
#include <wflow/queue/asio_queue.hpp>
#include "delayed_common_suite.hpp"

UNIT(asio_queue1, "")
{
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit1(t, *pq);
}


UNIT(asio_queue2, "")
{
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit2(t, *pq);
}

UNIT(asio_queue3, "")
{
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  delayed_unit3(t, *pq);
}

BEGIN_SUITE(asio_queue, "")
  ADD_UNIT(asio_queue1)
  ADD_UNIT(asio_queue2)
  ADD_UNIT(asio_queue3)
END_SUITE(asio_queue)

