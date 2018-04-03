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
  /**
   * @brief Использовать boost::asio::io_service (true) или wflow::delayed_queue
   * @deprecated Альтернативный io_service не дает существенных преимуществ
   */
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
   * @attention Не работает при workflow_options::use_io_service == false
   */

  size_t  threads = 0;

  
  /**
   * @brief Максимальный размер очереди заданий
   * @details При достижении этого размера, новые задания будут выкидываться из очереди. Если задан workflow_options::control_ms,
   * то в лог будут отображаться сообщение с ошибкой и количеством выкинутых заданий. В лог пишется не каждый факт потери задания,
   * а с переодичностью workflow_options::control_ms проверяются счетчики и в случае их изменения, делаеться запись. Таймеры и 
   * защищенные задания не выкидываются из очереди. 
   */
  size_t maxsize = 0;
  
  /**
   * @brief Размер очереди заданий, при достижении которого пишется предупреждение в лог
   * @details При достижении этого размера, если задан workflow_options::control_ms,
   * то в лог будут отображаться сообщение с предупреждением и количеством сообщений в очереди. В лог пишется не каждый факт превышения размера,
   * а с переодичностью workflow_options::control_ms проверяются счетчики и в случае их изменения, делаеться запись. Проверяется только количество
   * незащищенных заданий. Таймеры и защищенные задания не учитываються.
   */
  size_t wrnsize = 0;
  
  /**
   * @brief Интервал проверки счетчиков в миллисекундах
   * @details С этим интервалом проверяются счетчики workflow_options::maxsize и workflow_options::wrnsize для соответствующих записей в лог. 
   */
  time_t control_ms = 0;
  
  /**
   * @brief Общая задержка в миллисекундах 
   * @details Может использоваться для тестирования или как защита от bruteforce. Работает только для всех незащищенных заданий отправленных 
   * без временного интервала workflow_options::post
   */
  time_t post_delay_ms = 0; 
  
  /**
   * @brief Ограничение скорости обработки (в заданиях в секунду)
   * @details Может использоваться для тестирования как сервера так клиента. Работает только для всех незащищенных заданий 
   * (в том числе и отложенных) workflow_options::post
   */
  size_t rate_limit = 0; 

  /**
   * @brief Отладочный режим
   * @details Каждые workflow_options::control_ms выводит в лог размеры очередей и потери
   */
  bool debug = false;
  
  /**
    * @brief Указатель на workflow для таймера workflow_options::control_ms
    * @details при большом количестве тяжеловестных заданий таймер может не срабатывать во время,
    * поэтому для его рекомендуется создать отдельный workflow
    * @see example0.cpp 
    */
  std::shared_ptr<workflow> control_workflow;
  
  /**
   * @brief Альернативный обработчик для workflow_options::control_ms
   * @details Например, чтобы подавить любой вывод в лог:
   * ```cpp
   * workflow_options opt;
   * opt.control_handler=[](){return true;}
   * ```
   */
  std::function<bool()> control_handler = nullptr;
  
  /**
   * @brief тип обработчика для workflow_options::startup_handler
   * @param std::thread::id Идентификатор текущего потока
   */
  typedef std::function<void(std::thread::id)> startup_handler_t;
  
  /**
   * @brief тип обработчика для workflow_options::finish_handler
   * @param std::thread::id Идентификатор текущего потока
   */
  typedef std::function<void(std::thread::id)> finish_handler_t;
  
  /**
   * @brief Обработчик вызывается после запуска дополнительных потоков 
   * @see workflow_options::startup_handler_t, workflow_options::threads, workflow::reconfigure 
   */
  startup_handler_t startup_handler;
  
  /**
   * @brief Обработчик вызывается перед завершением дополнительных потоков 
   * @see workflow_options::finish_handler_t, workflow_options::threads, workflow::reconfigure 
   */
  finish_handler_t finish_handler;
  
  /** 
   * @brief тип временного интервала для статистики
   * @see workflow_options::statistics_handler_t, workflow_options::statistics_handler
   */
  typedef std::chrono::time_point<std::chrono::steady_clock>::duration statistics_duration;

  /** 
   * @brief тип функции обработчика статистики
   * @param std::thread::id Идентификатор текущего потока
   * @param size_t Количество выполненных заданий за интервал времени
   * @param statistics_duration интервал времени
   * @see workflow_options::statistics_handler
   */
  typedef std::function<void(std::thread::id, size_t count, statistics_duration)> statistics_handler_t;
  
  /**
   * @brief Обработчик для сбора статистики 
   * @details вызываеться на каждой итерации обработки очереди из каждого потока io_service::run()
   * @see workflow_options::statistics_handler_t
   */
  statistics_handler_t statistics_handler;
};

}
