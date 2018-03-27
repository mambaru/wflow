#pragma once
#include <memory>
#include <thread>
#include <string>
#include <functional>

namespace wflow{

class workflow;

struct workflow_options
{
  bool use_io_service = true;
  
  // Идентификатор для отображения в логк (не обязатльено)
  std::string id;
  
  // Количество дополнительных потоков обработки 
  size_t  threads = 0;

  // максимальный размер сообщений в очереди (отправленных post, но не safe_post )
  size_t maxsize = 512;
  // размер для предупреждения (отправленных post, но не safe_post )
  size_t wrnsize = 64;
  // интервал проверки размера очереди для логгирования
  time_t control_ms = 0;
  // общая задержка в миллисекундах для всех сообщений отправленных без временного интервала 
  time_t post_delay_ms = 0; 
  // ограничение скорости 
  size_t rate_limit = 0;    // ограничение скорости (в сообщениях в секунду)

  bool debug = false;
  
  // для таймера проверки wrnsize
  std::shared_ptr<workflow> control_workflow_ptr;
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
