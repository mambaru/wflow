#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>

namespace wflow{

class workflow;

/** 
 * @brief Опции
 */
struct workflow_options
{
  /// @deprecated При false использовать альтернативный io_service
  bool use_asio = true;
  
  /**
   * @brief Идентификатор для отображения в лог (не обязатльено)
   * @details Произвольная строка используеться при отображении в лог, 
   * чтобы можно было определить о каком workflow идет речь. 
   * 
   * @see workflow::get_id
   */
  std::string id;
  
  /** 
   * @brief Количество дополнительных потоков обработки 
   * @details Если значение 0 (по умолчанию), то никаких дополнительных потоков не запускается,
   * все сообщения и таймеры работают с asio::io_service, который был передан в конструкторе workflow::workflow.
   * При значениях больших 0, создается свой объект asio::io_service с которым и работают все дополнительные потоки.
   * Динамическое реконфигурирование потоков от 1..N работает ожидаемо, просто добавляются новые или завершают работу
   * (после выполнения текущего задания) лишние. Однако при переключении 0->N или N->0 потоков есть нюансы связанные с 
   * с тем, что в конфигурации N потоков используется свой объект asio::io_service. При переключении 0->N созданные 
   * тамеры и текущие задания остаются в пользовательском asio::io_service
   * * @attention Не работает при workflow_options::use_io_service == false
   */

  size_t  threads = 0;

  /// максимальный размер сообщений в очереди (отправленных post, но не safe_post )
  /**
   * @details управление размером очереди 
   */
  size_t maxsize = 0;
  
  /// размер для предупреждения (отправленных post, но не safe_post )
  size_t wrnsize = 0;
  
  /// интервал проверки размера очереди для логгирования
  time_t control_ms = 0;
  
  /// общая задержка в миллисекундах для всех сообщений отправленных без временного интервала 
  time_t post_delay_ms = 0; 
  
  /// ограничение скорости обработки
  size_t rate_limit = 0; 

  /// 
  bool debug = false;
  
  /// для таймера проверки wrnsize
  std::shared_ptr<workflow> control_workflow;
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
