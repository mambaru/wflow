#pragma once
#include <memory>
#include <thread>

namespace iow{

class workflow;

struct workflow_options
{
  std::string id;
  bool use_io_service = false;
  int  threads = 0;

  size_t maxsize = 1024*1024*128;
  size_t wrnsize = 1024*1024;
  time_t show_wrn_ms = 1000;

  time_t post_delay_ms = 0; // задержка в миллисекундах 
  size_t rate_limit = 0;    // ограничение скорости (в сообщениях в секунду)

  // для таймера проверки wrnsize
  std::shared_ptr< ::iow::workflow> workflow_ptr;
  std::function<bool()> handler = nullptr;
  
  typedef std::function<void(std::thread::id)> startup_handler_t;
  typedef std::function<void(std::thread::id)> finish_handler_t;
  
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler_t;
  startup_handler_t startup_handler;
  finish_handler_t finish_handler;
  statistics_handler_t statistics_handler;
};

}
