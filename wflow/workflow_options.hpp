#pragma once
#include <memory>
#include <thread>
#include <string>
#include <functional>

namespace wflow{

class workflow;

struct workflow_options
{
  std::string id;
  // в delayed_queue есть баг
  bool use_io_service = true;
  bool debug = false;
  size_t  threads = 0;

  size_t maxsize = 512;
  size_t wrnsize = 64;
  time_t control_ms = 0;

  time_t post_delay_ms = 0; // задержка в миллисекундах 
  size_t rate_limit = 0;    // ограничение скорости (в сообщениях в секунду)

  // для таймера проверки wrnsize
  std::shared_ptr< ::wflow::workflow> control_workflow_ptr;
  std::function<bool()> control_handler = nullptr;
  
  typedef std::function<void(std::thread::id)> startup_handler_t;
  typedef std::function<void(std::thread::id)> finish_handler_t;
  
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler_t;
  startup_handler_t startup_handler;
  finish_handler_t finish_handler;
  statistics_handler_t statistics_handler;
};

}
