#include <fas/testing.hpp>
#include <wflow/queue/delayed_queue.hpp>
#include <wflow/queue/asio_queue.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <cmath>
#include <condition_variable>

#include "delayed_common_suite.hpp"

UNIT(delayed_queue1, "")
{
  auto dq = std::make_shared<wflow::delayed_queue>(0);
  delayed_unit1(t, *dq);
}


UNIT(delayed_queue2, "")
{
  auto dq = std::make_shared<wflow::delayed_queue>(0);
  delayed_unit2(t, *dq);
}

UNIT(delayed_queue3, "")
{
  auto dq = std::make_shared<wflow::delayed_queue>(0);
  delayed_unit3(t, *dq);
}

UNIT(delayed_queue4, "")
{
  using namespace ::fas::testing;
  auto dq = std::make_shared<wflow::delayed_queue>(0);
  delayed_unit4(t, *dq);
}

BEGIN_SUITE(delayed_queue, "")
  ADD_UNIT(delayed_queue1)
  ADD_UNIT(delayed_queue2)
  ADD_UNIT(delayed_queue3)
  ADD_UNIT(delayed_queue4)
END_SUITE(delayed_queue)

