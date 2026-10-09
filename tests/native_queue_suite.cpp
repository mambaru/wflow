#include <fas/testing.hpp>
#include <wflow/queue/native_queue.hpp>
#include <wflow/queue/asio_queue.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <cmath>
#include <condition_variable>
#include <vector>

#include "delayed_common_suite.hpp"

namespace 
{
  
UNIT(native_queue1, "")
{
  auto dq = std::make_shared<wflow::native_queue>(0);
  delayed_unit1(t, *dq);
}

UNIT(native_queue2, "")
{
  auto dq = std::make_shared<wflow::native_queue>(0);
  delayed_unit2(t, *dq);
}

UNIT(native_queue3, "")
{
  auto dq = std::make_shared<wflow::native_queue>(0);
  delayed_unit3(t, *dq);
}

UNIT(native_queue_reset, "")
{
  using namespace ::fas::testing;
  auto dq = std::make_shared<wflow::native_queue>(0);
  size_t counter = 0;
  for (int i = 0; i < 10; ++i)
    dq->post([&counter]() noexcept { ++counter; }, nullptr);
  for (int i = 0; i < 5; ++i)
    dq->safe_post([&counter]() noexcept { ++counter; });

  t << equal<assert, size_t>(dq->unsafe_size(), 10) << FAS_FL;
  t << equal<assert, size_t>(dq->safe_size(), 5) << FAS_FL;

  dq->reset();

  t << equal<assert, size_t>(dq->unsafe_size(), 0) << FAS_FL;
  t << equal<assert, size_t>(dq->safe_size(), 0) << FAS_FL;
  t << equal<assert, size_t>(dq->full_size(), 0) << FAS_FL;
  t << equal<assert, size_t>(dq->dropped(), 0) << FAS_FL;
  t << is_false<assert>(dq->stopped()) << FAS_FL;

  dq->post([&counter]() noexcept { ++counter; }, nullptr);
  t << equal<assert>(dq->run_one(), 1) << FAS_FL;
  t << equal<expect, size_t>(counter, 1) << FAS_FL;
  t << equal<assert, size_t>(dq->unsafe_size(), 0) << FAS_FL;
}

UNIT(native_queue_drop_reenter, "")
{
  using namespace ::fas::testing;
  auto dq = std::make_shared<wflow::native_queue>(1);
  size_t done = 0;
  size_t drops = 0;

  t << is_true<assert>(dq->post([&done]() noexcept { ++done; }, nullptr)) << FAS_FL;
  // drop снова постит в ту же очередь — раньше это был deadlock под mutex
  t << is_false<assert>(dq->post(
    []() noexcept {},
    [&]() {
      ++drops;
      dq->safe_post([&done]() noexcept { ++done; });
    })) << FAS_FL;

  t << equal<assert, size_t>(drops, 1) << FAS_FL;
  t << equal<assert>(dq->run_one(), 1) << FAS_FL;
  t << equal<assert>(dq->run_one(), 1) << FAS_FL;
  t << equal<expect, size_t>(done, 2) << FAS_FL;
  t << equal<assert, size_t>(dq->dropped(), 1) << FAS_FL;
}

UNIT(native_queue_maxsize_safe, "")
{
  using namespace ::fas::testing;
  auto dq = std::make_shared<wflow::native_queue>(1, 1);
  // safe не должен занимать слот maxsize (как в asio_queue)
  dq->safe_post([]() noexcept {});
  dq->safe_post([]() noexcept {});
  t << is_true<assert>(dq->post([]() noexcept {}, nullptr)) << FAS_FL;
  t << is_false<assert>(dq->post([]() noexcept {}, nullptr)) << FAS_FL;
  t << equal<assert, size_t>(dq->safe_size(), 2) << FAS_FL;
  t << equal<assert, size_t>(dq->unsafe_size(), 1) << FAS_FL;
  t << equal<assert, size_t>(dq->dropped(), 1) << FAS_FL;
}

UNIT(native_queue_shards, "")
{
  using namespace ::fas::testing;
  auto dq = std::make_shared<wflow::native_queue>(0, 8);
  t << equal<assert, size_t>(dq->shard_count(), 8) << FAS_FL;

  std::atomic<size_t> done{0};
  const size_t producers = 8;
  const size_t per_prod = 1000;
  std::vector<std::thread> pubs;
  for (size_t i = 0; i < producers; ++i)
  {
    pubs.emplace_back([dq, &done]() {
      for (size_t j = 0; j < per_prod; ++j)
        dq->safe_post([&done]() noexcept { done.fetch_add(1, std::memory_order_relaxed); });
    });
  }

  std::thread worker([dq]() { dq->run(); });
  for (auto& pub : pubs)
    pub.join();

  while (done.load() < producers * per_prod)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  dq->stop();
  worker.join();
  t << equal<expect, size_t>(done.load(), producers * per_prod) << FAS_FL;
  t << equal<assert, size_t>(dq->full_size(), 0) << FAS_FL;
}

}

UNIT(native_queue_run_for_ms, "")
{
  using namespace fas::testing;
  using namespace std::chrono;

  auto q = std::make_shared<wflow::native_queue>(0);
  const auto t0 = steady_clock::now();
  const size_t n = q->run_for_ms(50);
  const auto ms = duration_cast<milliseconds>(steady_clock::now() - t0).count();
  t << equal<expect, size_t>(n, 0) << FAS_FL;
  t << greater_equal<expect, long long>(ms, 40) << FAS_FL;
  t << less<expect, long long>(ms, 250) << FAS_FL;
  q->stop();
}

BEGIN_SUITE(native_queue, "")
  ADD_UNIT(native_queue1)
  ADD_UNIT(native_queue2)
  ADD_UNIT(native_queue3)
  ADD_UNIT(native_queue_reset)
  ADD_UNIT(native_queue_drop_reenter)
  ADD_UNIT(native_queue_maxsize_safe)
  ADD_UNIT(native_queue_shards)
  ADD_UNIT(native_queue_run_for_ms)
END_SUITE(native_queue)

