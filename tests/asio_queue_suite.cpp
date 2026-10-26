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

UNIT(asio_queue_maxsize_safe, "")
{
  using namespace ::fas::testing;
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 1);

  // safe не занимает слот maxsize
  pq->safe_post([]() noexcept {});
  pq->safe_post([]() noexcept {});
  t << is_true<assert>(pq->post([]() noexcept {}, nullptr)) << FAS_FL;
  t << is_false<assert>(pq->post([]() noexcept {}, nullptr)) << FAS_FL;
  t << equal<assert, size_t>(pq->safe_size(), 2) << FAS_FL;
  t << equal<assert, size_t>(pq->unsafe_size(), 1) << FAS_FL;
  t << equal<assert, size_t>(pq->dropped(), 1) << FAS_FL;
}

UNIT(asio_queue_drop_reenter, "")
{
  using namespace ::fas::testing;
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 1);
  size_t done = 0;
  size_t drops = 0;

  t << is_true<assert>(pq->post([&done]() noexcept { ++done; }, nullptr)) << FAS_FL;
  // drop снова постит в ту же очередь — не должен зависать
  t << is_false<assert>(pq->post(
    []() noexcept {},
    [&]() {
      ++drops;
      pq->safe_post([&done]() noexcept { ++done; });
    })) << FAS_FL;

  t << equal<assert, size_t>(drops, 1) << FAS_FL;
  t << equal<assert, size_t>(pq->dropped(), 1) << FAS_FL;
  io.run();
  t << equal<expect, size_t>(done, 2) << FAS_FL;
}

UNIT(asio_queue_stop_restart, "")
{
  using namespace ::fas::testing;
  boost::asio::io_context io;
  auto pq = std::make_shared< ::wflow::asio_queue >(io, 0);
  size_t done = 0;

  pq->post([&done]() noexcept { ++done; }, nullptr);
  t << equal<assert>(pq->run_one(), 1) << FAS_FL;
  t << equal<expect, size_t>(done, 1) << FAS_FL;

  pq->stop();
  t << is_true<assert>(pq->stopped()) << FAS_FL;

  // reset = io.restart: очередь снова можно крутить (handlers не вычищаются — семантика asio)
  pq->reset();
  t << is_false<assert>(pq->stopped()) << FAS_FL;

  pq->post([&done]() noexcept { ++done; }, nullptr);
  t << equal<assert>(pq->run_one(), 1) << FAS_FL;
  t << equal<expect, size_t>(done, 2) << FAS_FL;
}

BEGIN_SUITE(asio_queue, "")
  ADD_UNIT(asio_queue1)
  ADD_UNIT(asio_queue2)
  ADD_UNIT(asio_queue3)
  ADD_UNIT(asio_queue_maxsize_safe)
  ADD_UNIT(asio_queue_drop_reenter)
  ADD_UNIT(asio_queue_stop_restart)
END_SUITE(asio_queue)
