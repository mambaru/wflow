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
   * @brief Использовать wflow::native_queue вместо asio
   * @details false (по умолчанию) — boost::asio::io_context / asio_queue.
   * true — native_queue; обычно быстрее на легковесных заданиях (см. example17).
   * Asio удобнее, если в том же io_context уже живут сокеты/таймеры.
   */
  bool use_native = false;

  /**
   * @brief Идентификатор для отображения в лог (не обязатльено)
   * @details Произвольная строка используется при отображении в лог,
   * чтобы можно было определить о каком workflow идет речь.
   *
   * @see workflow::get_id
   */
  std::string id;

  /**
   * @brief Число потоков пула (только пул; очередь всегда жива)
   * @details
   * - `0` — пула нет. Задания крутятся через `io_context`:
   *   ctor с внешним `io` → этот `io`; ctor без `io` → внутренний (`workflow::get_io_context()`).
   * - `>0` — свой пул. При ctor без внешнего `io` у пула свой `io_context`.
   * Reconfigure всегда успешен: `1..N↔M` мягко; переход через `0` или смена `use_native` —
   * со сбросом (wrap → alt-handler).
   */

  size_t  threads = 0;


  /**
   * @brief Максимальный размер очереди незащищённых заданий
   * @details При достижении лимита новые `post` отбрасываются (drop). `0` — без лимита:
   * очередь может расти без верхней границы (риск исчерпания памяти под нагрузкой).
   * Таймеры и `safe_post` лимит не занимают. При заданном `control_ms` в лог пишутся
   * потери с этой периодичностью, а не на каждый drop.
   */
  size_t maxsize = 100000;

  /**
   * @brief Размер очереди заданий, при достижении которого пишется предупреждение в лог
   * @details При достижении этого размера, если задан workflow_options::control_ms,
   * то в лог будут отображаться сообщение с предупреждением и количеством сообщений в очереди. В лог пишется не каждый факт превышения размера,
   * а с переодичностью workflow_options::control_ms проверяются счетчики и в случае их изменения, делается запись. Проверяется только количество
   * незащищенных заданий. Таймеры и защищенные задания не учитываются.
   */
  size_t wrnsize = 50000;

  /**
   * @brief Интервал проверки счетчиков в миллисекундах
   * @details С этим интервалом проверяются счетчики workflow_options::maxsize и workflow_options::wrnsize для соответствующих записей в лог.
   * Если control_ms=0, то отключен
   */
  time_t control_ms = 5000;

  /**
    * @brief Как часто рабочий поток «отрывается» от очереди (мс)
    * @details Между порциями заданий поток ненадолго возвращает управление:
    * можно уменьшить число threads или остановить workflow без зависания.
    * Если задан обработчик статуса — он вызывается с этим же интервалом.
    * При status_ms=0 обработчик статуса не вызывается, а интервал пробуждения
    * остаётся 1с (иначе reconfigure/stop могут зависнуть).
    */
  time_t status_ms = 1000;

  /**
   * @brief Тихий режим. Отключает вывод лога ошибок при переполнении
   * @details Не отключает вывода информации о переполнении control_ms
   */
  bool quiet_mode = false;

  /**
   * @brief Полный сброс очереди при переполнении
   * @details Если maxsize установлено в такое значение, что разбор всей очереди не целесообразен, то имеет смысл ее сбрасывать при каждом переполнении.
   */
  bool overflow_reset = false;

  /**
   * @brief Общая задержка в миллисекундах
   * @details Может использоваться для тестирования или как защита от bruteforce. Работает только для всех незащищенных заданий отправленных
   * без временного интервала workflow_options::post
   */
  time_t post_delay_ms = 0;

  /**
   * @brief Ограничение скорости приёма незащищённых заданий (в post/сек)
   * @details Первые rate_limit заданий в окне уходят в очередь сразу, остальные
   * планируются равномерно (слот i/rate_limit сек от начала окна), без drop.
   * Для тестирования и защиты от bruteforce. Только workflow_options::post
   * (в т.ч. через delayed/post_at); safe_post не ограничивается.
   */
  size_t rate_limit = 0;

  /**
   * @brief Отладочный режим
   * @details Каждые workflow_options::control_ms выводит в лог размеры очередей и потери
   */
  bool debug = false;
};

struct workflow_handlers
{
  /**
    * @brief Указатель на workflow для таймера workflow_options::control_ms
    * @details при большом количестве тяжеловесных заданий таймер может не срабатывать во время,
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
   * @brief тип обработчика для startup_handler
   * @param std::thread::id Идентификатор текущего потока
   */
  typedef std::function<void(std::thread::id)> startup_handler_t;


  /**
   * @brief тип обработчика для status_handler
   * @param std::thread::id Идентификатор текущего потока
   */
  typedef std::function<void(std::thread::id)> status_handler_t;

  /**
   * @brief тип обработчика для finish_handler
   * @param std::thread::id Идентификатор текущего потока
   */
  typedef std::function<void(std::thread::id)> finish_handler_t;

  /**
   * @brief Обработчик вызывается после запуска дополнительных потоков
   * @see workflow_options::startup_handler_t, workflow_options::threads, workflow::reconfigure
   */
  startup_handler_t startup_handler = nullptr;

  /**
   * @brief Обработчик вызывается в процессе выполенния потоков между заданиями
   * @see workflow_options::startup_handler_t, workflow_options::threads, workflow::reconfigure
   */
  status_handler_t status_handler = nullptr;

  /**
   * @brief Обработчик перед выходом рабочего потока пула
   * @details Нельзя вызывать workflow::stop / wait / reconfigure — join этого же
   * потока из finish_handler приведёт к deadlock.
   * @see workflow_options::finish_handler_t, workflow_options::threads, workflow::reconfigure
   */
  finish_handler_t finish_handler = nullptr;

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
   * @details вызывается на каждой итерации обработки очереди из каждого потока io_context::run()
   * @see workflow_options::statistics_handler_t
   */
  statistics_handler_t statistics_handler = nullptr;
};

}
