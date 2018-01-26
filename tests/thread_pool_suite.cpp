#include <fas/testing.hpp>
#include <wflow/delayed_queue.hpp>
#include <wflow/thread_pool.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <cmath>
#include <condition_variable>

UNIT(thread_pool1, "")
{
  using namespace ::fas::testing;
  using namespace ::std::chrono;
  t << nothing;
}

BEGIN_SUITE(thread_pool, "")
  ADD_UNIT(thread_pool1)
END_SUITE(thread_pool)

