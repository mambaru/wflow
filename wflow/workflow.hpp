#pragma once

#include <wflow/workflow_options.hpp>
#include <wflow/expires_at.hpp>
#include <wflow/task/task_manager.hpp>
#include <wflow/system/asio.hpp>
#include <chrono>

namespace wflow{

/**
 * @example example0.cpp нулевой пример
 * Пример того, как не надо жить
 */

/**
 * @brief Очередь заданий с поддержкой отложенных заданий, таймеров, пула потоков, динамической реконфигурацией и конфигурируемыми ограничениями на размер очереди и скорость обработки.
 * @details *workflow* можно рассматривать как надстройку над boost::asio::io_service. Существуют четыре группы заданий:
 * * Защищенные задания 
 * * Незащищенные задания 
 * * Обычные таймеры
 * * Асинхронные таймеры
 * * Опросники (requester)
 * 
 * ## Защищенные задания ( workflow::safe_post )
 * 
 * Обработчик защищенного задания гарантированно не будут выкинут из очереди, даже при ее переполнении. Таймеры также используют этот вид заданий. 
 * Отложенные защищенные задания выполняються в момент срабатывания таймера, в отличие от незащищенных, которые в этот момент только отправят задание 
 * в общую очередь на обработку. Кроме того защищенные задания игнорируют общие ограничения по задержке и скорости обработки, которые задаються в конфигурации.
 * 
 * В общем случае, если нет необходимости использовать опциональные ограничения, то рекомендуется использовать этот вид заданий, т.к. нет дополнительных накладных расходов. 
 * Однако в нагруженных системах рекомендуется использовать отдельные wflow::workflow для тамеров и основного потока заданий, чтобы большое количество заданий не мешало отрабатывать 
 * таймерам во время. Т.к. отложеные задание также используют таймеры, в опциях можно передать указатель на wflow::workflow специально для этих целей. См. \ref example0.cpp
 * 
 * ## Незащищенные задания ( workflow::post )
 * Этот тип заданий может быть выкинут из очереди заданий (с возможностью вызова альтернативного обработчика), а также на очередь таких заданий можно наложить ограничения 
 * на скорость обработки и/или установить задержку. Если указать альтернативный обработчик при отправке задания в очередь, то он сработает в случае переполнения очереди, если 
 * основной обработчик был выкинут из очереди. \ref example1.cpp Если workflow::post возвращает false, то однозначно заданию нет места в очереди, и альтернативный обработчик 
 * был выполнен, если true -  * то задание было поставленно в очередь, но не факт, что будет выполненно. 
 * 
 * Если не задано ограничений на размер очереди (по умолчанию), то задания не будут выкидываться из очерди, но отработка отложеных заданий и/или при ограничении скорости обработки
 * отличается. В то время как у защищенного отложенного задания тамер устанавливается на время выполнения обработчика задания, то у незащенного задания - на время отправки в общую 
 * очередь. Т.е. задание на отправку в очередь, в заданное время, будет выполненно, но не факт, что успешно, если очередь будет переполненна. \ref example2.cpp 
 * 
 * Незащищенные задания имеет смысл использовать на высоких на грузках, когда есть вероятность того, что система не будет справляться с потоком заданий, чтобы не допустить 
 * разбухания очереди workflow_options::maxsize. Например в сетевых приложениях, через незащищенную очередь пропускают все запросы клиентов. Это может быть удобно, также, 
 * для тестирования, например искуственно увеличив время ответа workflow_options::post_delay_ms или ограничив пропускную способность workflow_options::rate_limit
 * 
 * ## Обычные таймеры (workflow::create_timer)
 * При создании таймера задается функция (обработчик таймера workflow::timer_handler) которая выполняеться с заданым интервалом времени. Обработчик таймера должен возвращать true, 
 * если тамер еще актуален и false - если он больше не нужен. По мимо интервала выполнения, можно передать флаг определяющий, как будет отсчитываться следующий момен времени вызова 
 * обработчика: с начала или после выполнения. Также можно задать время первого выполнения. Напремер "через 10 секунд и далее каждую секунду" \ref exampleX.cpp или в пять утра 
 * и далее каждые 6 часов. Как уже было сказано, при больших нагрузках (например в сетевых приложениях) не рекомендуется использовать таймеры в общем wflow::workflow (ну или наоборот, 
 * для потока запросов создать отдельный wflow::workflow)
 * 
 * ## Асинхронные таймеры (workflow::create_asyn_timer)
 * Отличаеться от обычного таймера тем, что в обработчик таймера workflow::async_timer_handler пердается функция обратного вызова workflow::callback_timer_handler, которую он должен 
 * вызвать, чтобы сигнализировать о том, что обработка завершена и что делать далее: завершить работу таймера или продолжить. Используется если в обработчике таймера нужно выполнить 
 * асинхронную операцию, например сделать запрос на удаленный сервер.
 * 
 * ## Опросники (requester)
 * Вид асинхронных таймеров 
 * 
 * ```cpp
 * struct request{...};
 * struct response{...};
 * class ifoo
 * {
 *   virtual void method( std::unique_ptr<request> req, std::function<void(std::unique_ptr<response>) >) = 0;
 * }
 * ```
 * 
 */
class workflow
{
public:
  /// Переопределение boost::asio::io_service 
  typedef ::wflow::asio::io_service io_service_type;
  /// Обработчик задания
  typedef std::function< void() > post_handler;
  /// Обработчик сброса задания
  typedef std::function< void() > drop_handler;
  /// Обработчик таймера (возвращает false для остановки таймера)
  typedef std::function<bool()> timer_handler;
  /// Калбэк для обработчика асинхронного таймера
  typedef std::function<void(bool)> callback_timer_handler;
  /// Обработчик асинхронного таймера (вызывает callback_timer_handler)
  typedef std::function<void(callback_timer_handler)> async_timer_handler;
  /// Стандартные часы
  typedef std::chrono::system_clock         clock_t;
  /// Момент времени
  typedef std::chrono::time_point<clock_t>  time_point_t;
  /// Интервал времени
  typedef time_point_t::duration            duration_t;
  /// Идентификатор таймера
  typedef int                               timer_id_t;
  
  typedef task_manager::timer_type          timer_type;
  
public:
  virtual ~workflow();
  workflow(const workflow& ) = delete;
  workflow& operator=(const workflow& ) = delete;
  
  /**
   * @brief Конструктор без поддержки однопоточного режима.
   * @details Для работы нужен как минимум один дополнительный поток. Для этого в опциях 
   * необходимо указать workflow_options::threads = 1 или больше. При workflow_options::threads == 0 задачи из очереди обрабатываться не будут.
   * @remark По умолчанию в workflow_options::threads = 0 - необходимо изменить это значение.
   * @param opt - опции. 
   */
  explicit workflow(const workflow_options& opt);
  
  /**
   * @brief Основной конструктор с поддержкой различных режимов.
   * @details Может работать как в основном потоке приложения, так и создавать
   * свой пул потоков. При workflow_options::threads == 0 дополнительные потоки не создаются о обработка 
   * происходит при вызове io.run() или аналогичных. Для работы в многопоточном режиме
   * укажите количество потоков. При workflow_options::threads == 1 создается один поток и т.д. При многопоточном 
   * режиме io не используется, а создаеться свой (один на все потоки этого объекта).
   * @remark При workflow_options::threads == 1 или большем создается свой объект io_service, а io не используется 
   * @param io - ссылка на boost::asio::io_service.
   * @param opt - опции. 
   */
  workflow(io_service_type& io, const workflow_options& opt = workflow_options() );
  
  /**
   * @brief Запуск потоков обработки.
   * @details Работает только в многопоточном режиме. При threads=0 ничего не происходит. Может 
   * использовтаться для возобновления работы после вызова workflow::stop.
   */
  void start();
  
  /**
   * @brief Реконфигурация после запуска.
   * @details Позволяет реконфигурировать workflow без сброса очередей. 
   * Можно изменить число потоков или ограничения очереди.
   * 
   * @attention Не работает при workflow_options::use_io_service == false
   * @param opt - новые опции. 
   */
  bool reconfigure(const workflow_options& opt);
  
  void clear();

  /**
   * Остановка потоков в многопоточном режиме.
   */
  void stop();
  
  /**
   * Отправить задание на обработку
   * @param handler обработчик задания типа void()
   * @param drop альтернативный обработчик, если задание выкидывается из очереди. По умолчанию nullptr (handler выкидывается из очереди без уведомления)
   * @return true - задание поставлено в очередь обработки. false - очередь переполнена, обработчик drop вызван
   * 
   * При переполнении очереди drop может быть вызван как синхронно (чаще всего) так и асинхронно (если установлена опция rate_limit)
   */
  bool post(post_handler handler, drop_handler drop = nullptr);
  
  /**
   * Отправить задание на обработку в указанный момент времени
   * @param tp момент времени, в который задание будет отправлено на обработку
   * @param handler обработчик задания типа void()
   * @param drop альтернативный обработчик, если задание выкидывается из очереди. По умолчанию nullptr (handler выкидывается из очереди без уведомления)
   * @return true - включен таймер для отправки сообщения. Таймер выкинут из очереди быть не может поэтому всегда true
   * 
   * В момент времени tp задание будет только отправлено в очередь на обработку (и может быть выкинуто, при переполнении ), 
   * в отличии от safe_post, где оно будет выполнено сразу.
   */
  bool post(time_point_t tp, post_handler handler, drop_handler drop = nullptr);
  
  /**
   * Отправить задание на обработку через указанный интервал времени
   * @param duration интервал времени, через который задание будет отправлено на обработку
   * @param handler обработчик задания типа void()
   * @param drop альтернативный обработчик, если задание выкидывается из очереди. По умолчанию nullptr (handler выкидывается из очереди без уведомления)
   * @return true - включен таймер для отправки сообщения. Таймер выкинут из очереди быть не может поэтому всегда true
   * 
   * Через указанный интервал времени duration задание будет только отправлено в очередь на обработку (и может быть выкинуто, при переполнении ), 
   * в отличии от safe_post, где оно будет выполнено сразу.
   */
  bool post(duration_t duration, post_handler handler, drop_handler drop = nullptr);

  /**
   * Отправить задание на обработку игнорируя ограничения на размер очереди и скорости обработки.
   * @param handler обработчик задания типа void()
   * 
   * Аналог boost::asio::io_service::post
   */
  void safe_post(post_handler handler);
  
  /**
   * Выполнить задание в указанный момент времени
   * @param tp момент времени, в который задание будет выполнено
   * @param handler обработчик задания типа void()
   */
  void safe_post(time_point_t tp, post_handler handler);

  /**
   * Выполнить задание через указанный интервал времени
   * @param duration интервал времени, через который задание будет выполнено
   * @param handler обработчик задания типа void()
   */
  void safe_post(duration_t duration, post_handler handler);

  /**
   * Создает таймер, обработчик которого выполняется с заданной периодичностью
   * @param duration интервал таймера 
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(duration_t duration, timer_handler handler, expires_at expires = expires_at::after);
  
  /**
   * Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью
   * @param duration интервал таймера 
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true), 
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера (вызова callback_timer_handler из обработчика)
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * Создает таймер, обработчик которого выполняется с заданной периодичностью через определенный интервал времени 
   * @param start_duration интервал времени, через который запустить таймер
   * @param duration интервал таймера 
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(duration_t statrt_duration, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);
  
  /**
   * Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью через определенный интервал времени 
   * @param start_duration интервал времени, через который запустить таймер
   * @param duration интервал таймера 
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true), 
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера (вызова callback_timer_handler из обработчика)
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(duration_t start_duration, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * Создает таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер
   * @param duration интервал таймера 
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(time_point_t tp, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер
   * @param duration интервал таймера 
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true), 
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера (вызова callback_timer_handler из обработчика)
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(time_point_t tp, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /** Создает таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param duration интервал таймера 
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(std::string tp, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);
  
  /**
   * Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param duration интервал таймера 
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true), 
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера (вызова callback_timer_handler из обработчика)
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(std::string tp, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);


  /** Создает таймер, обработчик которого выполняется раз в сутки начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(std::string tp, timer_handler handler, expires_at expires = expires_at::after);
  
  /**
   * Создает асинхронный таймер, обработчик которого выполняется раз в сутки с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true), 
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера (вызова callback_timer_handler из обработчика)
   * 
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(std::string tp, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * Остановить таймер и освободить ресурсы
   * @param id, идентификатор таймера
   * @return bool, если true - таймер остановлен, false - таймер с таким id не найдено  
   */
  bool release_timer( timer_id_t id );
  
  /**
   * Отсоединить таймер. Таймер продолжает работать до тех пор пока существует объект, который вернул detach_timer
   * @param id - идентификатор таймера
   * @return std::shared_ptr<bool>, если true - таймер работает , false - таймер с таким id не найдено
   * 
   * простой вызов detach_timer эквивалентен release_timer, но если std::shared_ptr<bool> t = detach_timer(id), то таймер будет продолжать работать 
   * до тех пор, пока t.reset() или t = nullptr
   */
  std::shared_ptr<bool> detach_timer(timer_id_t id);
  
  /* Общее количество таймеров */
  size_t timer_count() const;

  /* Общее количество обработчиков в очереди (включая таймеры) */
  size_t full_size() const;

  /* Общее количество невыкидываемых обработчиков в очереди (включая таймеры) */
  size_t safe_size() const;

  /* Количество обработчиков в очереди, которые могут быть выкинуты при переполнении */
  size_t unsafe_size() const;

  /* Общее количество сброшенных обработчиков при переполнении */
  size_t dropped() const;
  
  template< typename Req, typename Res, typename I, typename MemFun, typename Handler >
  timer_id_t create_requester( duration_t d, std::shared_ptr<I> i, MemFun mem_fun, Handler handler )
  {
    return this->get_timer()->create<Req, Res>( d, i, mem_fun, std::move(handler) );
  }

  template< typename Req, typename Res, typename I, typename MemFun, typename Handler >
  timer_id_t create_requester( duration_t sd, duration_t d, std::shared_ptr<I> i, MemFun mem_fun, Handler handler )
  {
    return this->get_timer()->create<Req, Res>( sd, d, i, mem_fun, std::move(handler) );
  }

  template< typename Req, typename Res, typename I, typename MemFun, typename Handler >
  timer_id_t create_requester( std::string st, duration_t d, std::shared_ptr<I> i, MemFun mem_fun, Handler handler )
  {
    return this->get_timer()->create<Req, Res>(st, d, i, mem_fun, std::move(handler) );
  }

  template< typename Req, typename Res, typename I, typename MemFun, typename Handler >
  timer_id_t create_requester( time_point_t tp, duration_t d, std::shared_ptr<I> i, MemFun mem_fun, Handler handler )
  {
    return this->get_timer()->create<Req, Res>( tp, d, i, mem_fun, std::move(handler) );
  }


  std::shared_ptr<task_manager> manager() const;
  std::shared_ptr<timer_type> get_timer() const;

private:
  void create_wrn_timer_(const workflow_options& opt);
  void initialize_(const workflow_options& opt);
private:
  std::string _id;
  std::atomic<time_t> _delay_ms;
  std::shared_ptr<task_manager> _impl;
  timer_id_t _wrn_timer = 0;
  std::shared_ptr<workflow> _workflow_ptr;
};



}
