#include <wflow/queue/asio_queue.hpp>
#include <wflow/queue/native_queue.hpp>
#include <wflow/workflow.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

/**
 * @example example17.cpp
 * @brief Бенчмаркинг asio_queue vs native_queue на пустых заданиях.
 *
 * Usage:
 *   example17 [seconds=3] [consumers=1] [producers=1]
 *
 * Сравнивает:
 *   1) очередь напрямую (asio_queue / native_queue)
 *   2) workflow с use_native=false/true (полный путь через bique)
 */

namespace {

using clock_t = std::chrono::steady_clock;

struct result
{
  const char* name;
  size_t posted;
  size_t executed;
  double seconds;
  size_t dropped;
};

void print_result(const result& r)
{
  const double post_rate = r.seconds > 0.0 ? double(r.posted) / r.seconds : 0.0;
  const double exec_rate = r.seconds > 0.0 ? double(r.executed) / r.seconds : 0.0;
  std::cout << std::left << std::setw(22) << r.name
            << " posted=" << std::setw(12) << r.posted
            << " exec=" << std::setw(12) << r.executed
            << " drop=" << std::setw(8) << r.dropped
            << " post/s=" << std::setw(12) << std::fixed << std::setprecision(0) << post_rate
            << " exec/s=" << exec_rate
            << std::endl;
}

template<typename Queue>
result bench_queue(
  const char* name,
  std::shared_ptr<Queue> q,
  size_t consumers,
  size_t producers,
  double seconds,
  std::function<void()> start_extra = {},
  std::function<void()> stop_extra = {})
{
  std::atomic<bool> run{true};
  std::atomic<size_t> posted{0};
  std::atomic<size_t> executed{0};

  if (start_extra)
    start_extra();

  std::vector<std::thread> workers;
  workers.reserve(consumers);
  for (size_t i = 0; i < consumers; ++i)
  {
    workers.emplace_back([q]() {
      q->run();
    });
  }

  // дать воркерам зайти в run()
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  std::vector<std::thread> pubs;
  pubs.reserve(producers);
  for (size_t i = 0; i < producers; ++i)
  {
    pubs.emplace_back([&, q]() {
      while (run.load(std::memory_order_relaxed))
      {
        q->safe_post([&executed]() noexcept {
          executed.fetch_add(1, std::memory_order_relaxed);
        });
        posted.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  const auto t0 = clock_t::now();
  std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
  run = false;
  const auto t1 = clock_t::now();

  for (auto& t : pubs)
    t.join();

  // дождаться хвоста и остановить
  while (q->full_size() > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  q->stop();
  if (stop_extra)
    stop_extra();

  for (auto& t : workers)
    t.join();

  const double elapsed = std::chrono::duration<double>(t1 - t0).count();
  return result{name, posted.load(), executed.load(), elapsed, q->dropped()};
}

result bench_asio(size_t consumers, size_t producers, double seconds)
{
  auto io = std::make_shared<boost::asio::io_context>();
  auto work = std::make_shared<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
    io->get_executor());
  auto q = std::make_shared<wflow::asio_queue>(*io, 0);
  return bench_queue(
    "asio_queue",
    q,
    consumers,
    producers,
    seconds,
    {},
    [work, io]() {
      work->reset();
      io->stop();
    });
}

result bench_native(size_t consumers, size_t producers, double seconds)
{
  auto q = std::make_shared<wflow::native_queue>(0);
  return bench_queue("native_queue", q, consumers, producers, seconds);
}

result bench_workflow(bool use_native, size_t consumers, size_t producers, double seconds)
{
  wflow::workflow_options opt;
  opt.use_native = use_native;
  opt.threads = consumers;
  opt.id = use_native ? "wf-native" : "wf-asio";
  wflow::workflow wf(opt);
  wf.start();

  std::atomic<bool> run{true};
  std::atomic<size_t> posted{0};
  std::atomic<size_t> executed{0};

  std::vector<std::thread> pubs;
  pubs.reserve(producers);
  for (size_t i = 0; i < producers; ++i)
  {
    pubs.emplace_back([&]() {
      while (run.load(std::memory_order_relaxed))
      {
        wf.safe_post([&executed]() noexcept {
          executed.fetch_add(1, std::memory_order_relaxed);
        });
        posted.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  const auto t0 = clock_t::now();
  std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
  run = false;
  const auto t1 = clock_t::now();

  for (auto& t : pubs)
    t.join();

  // shutdown/wait завязаны на asio work_guard; для native нужен явный stop()
  while (wf.full_size() > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  wf.stop();

  const double elapsed = std::chrono::duration<double>(t1 - t0).count();
  const char* name = use_native ? "workflow native" : "workflow asio";
  return result{name, posted.load(), executed.load(), elapsed, wf.dropped()};
}

} // namespace

int main(int argc, char const* const argv[])
{
  double seconds = 3.0;
  size_t consumers = 1;
  size_t producers = 1;

  if (argc > 1)
    seconds = std::atof(argv[1]);
  if (argc > 2)
    consumers = size_t(std::atol(argv[2]));
  if (argc > 3)
    producers = size_t(std::atol(argv[3]));
  if (seconds <= 0.0)
    seconds = 3.0;
  if (consumers == 0)
    consumers = 1;
  if (producers == 0)
    producers = 1;

  auto probe = std::make_shared<wflow::native_queue>(0);
  std::cout << "bench: seconds=" << seconds
            << " consumers=" << consumers
            << " producers=" << producers
            << " native_shards=" << probe->shard_count()
            << " (safe_post empty handler, maxsize=0)\n"
            << std::endl;

  std::cout << "=== raw queues ===" << std::endl;
  print_result(bench_asio(consumers, producers, seconds));
  print_result(bench_native(consumers, producers, seconds));

  std::cout << "\n=== workflow (bique) ===" << std::endl;
  print_result(bench_workflow(false, consumers, producers, seconds));
  print_result(bench_workflow(true, consumers, producers, seconds));

  return 0;
}
