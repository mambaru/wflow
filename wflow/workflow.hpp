#pragma once

#include <wflow/workflow_options.hpp>
#include <wflow/expires_at.hpp>
#include <wflow/task/task_manager.hpp>
#include <wflow/system/asio.hpp>
#include <chrono>

namespace wflow{

/**
   @brief Очередь заданий с поддержкой отложенных заданий, таймеров, пула потоков, динамической реконфигурацией и конфигурируемыми ограничениями на размер очереди и скорость обработки.
   @details *workflow* можно рассматривать как надстройку над boost::asio::io_context. Существуют пять типов заданий:
      - Защищенные задания
      - Незащищенные задания
      - Обычные таймеры
      - Асинхронные таймеры
      - Опросники (requester)

   ## Защищенные задания ( workflow::safe_post )
   Обработчик защищенного задания гарантированно не будут выкинут из очереди, даже при ее переполнении. Таймеры также используют этот вид заданий.
   Отложенные защищенные задания выполняются в момент срабатывания таймера, в отличие от незащищенных, которые в этот момент только отправят задание
   в общую очередь на обработку. Кроме того защищенные задания игнорируют общие ограничения по задержке и скорости обработки, которые задаются в конфигурации.
```cpp
boost::asio::io_context ios;
wflow::workflow wf(ios);

//// Простое задание
wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );

//// Отложенное задание
wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );
```
   В общем случае, если нет необходимости использовать ограничения, то рекомендуется использовать этот вид заданий, т.к. нет дополнительных накладных расходов.
   Однако в нагруженных системах рекомендуется использовать отдельные wflow::workflow для таймеров и основного потока заданий, чтобы большое количество заданий не мешало отрабатывать
   таймерам во время. Т.к. отложенные задание также используют таймеры, в опциях можно передать указатель на wflow::workflow специально для этих целей.
   См. \ref example01.cpp, \ref example11.cpp, \ref example12.cpp

   ## Незащищенные задания ( workflow::post )
   Этот тип заданий может быть выкинут из очереди (с возможностью вызова альтернативного обработчика), а также на очередь таких заданий можно наложить ограничения
   на скорость обработки и/или установить задержку. Если указать альтернативный обработчик при отправке задания в очередь, то он сработает в случае переполнения очереди, если
   основной обработчик был выкинут из очереди. Если workflow::post возвращает false, то однозначно заданию нет места в очереди, и альтернативный обработчик
   был выполнен, если true -  * то задание было поставлено в очередь, но не факт, что будет выполнено.
   См. \ref example02.cpp, \ref example03.cpp, \ref example04.cpp

```cpp
boost::asio::io_context ios;
wflow::workflow wf(ios);

//// Простое задание
wf.post( [](){ std::cout << "Simple unsafe post  " << std::endl; } );

//// Отложенное задание
wf.post( std::chrono::seconds(4), [](){ std::cout << "Safe unpost after delay 4 second " << std::endl; } );
```

   Если не задано ограничений на размер очереди (по умолчанию), то задания не будут выкидываться из очереди, но отработка отложенных заданий и/или при ограничении скорости обработки
   отличается. В то время как у защищенного отложенного задания таймер устанавливается на время выполнения обработчика задания, то у незащищенного задания - на время отправки в общую
   очередь. Т.е. задание на отправку в очередь, в заданное время, будет выполнено, но не факт, что успешно, если очередь будет переполнена. \ref example04.cpp

   Незащищенные задания имеет смысл использовать на высоких нагрузках, когда есть вероятность того, что система не будет справляться с потоком заданий, чтобы не допустить
   разбухания очереди workflow_options::maxsize. Например в сетевых приложениях, через незащищенную очередь пропускают все запросы клиентов. Это может быть удобно, также,
   для тестирования, например искусственно увеличив время ответа workflow_options::post_delay_ms или ограничив пропускную способность workflow_options::rate_limit

   ## Обычные таймеры (workflow::create_timer)
   При создании таймера задается функция (обработчик таймера workflow::timer_handler) которая выполняется с заданным интервалом времени. Обработчик таймера должен возвращать true,
   если таймер еще актуален и false - если он больше не нужен. Помимо интервала выполнения, можно передать флаг определяющий, как будет отсчитываться следующий момент времени вызова
   обработчика: с начала или после выполнения. Также можно задать время первого выполнения. Например "через 10 секунд и далее каждую секунду" \ref example05.cpp или в пять утра
   и далее каждые 6 часов. Как уже было сказано, при больших нагрузках (например в сетевых приложениях) не рекомендуется использовать таймеры в общем wflow::workflow (ну или наоборот,
   для потока запросов создать отдельный wflow::workflow)

   ## Асинхронные таймеры (workflow::create_asyn_timer)
   Отличаются от обычного таймера тем, что в обработчик таймера workflow::async_timer_handler пердается функция обратного вызова workflow::callback_timer_handler, которую он должен
   вызвать, чтобы сигнализировать о том, что обработка завершена и что делать далее: завершить работу таймера или продолжить. Используется если в обработчике таймера нужно выполнить
   асинхронную операцию, например сделать запрос на удаленный сервер.

   ## Опросники (requester)
   Вид асинхронных таймеров, предназначенных для создания последовательности запросов на основе пришедших ответов. Типичные примеры это периодическое обновление данных из БД или другого
   сервиса с помощью нескольких запросов, реализация репликации и пр. Для реализации потребуются структуры запроса и ответа, обработчики отправителя и генератора запросов.

   Запросом и ответом может быть любой тип, но предпочтительно использовать структуры с определенными типами для удобства использования:
```cpp
struct request
{
  ...
////исключительно для удобства
  typedef std::unique_ptr<request> ptr;
};

struct response
{
  ...
  ////исключительно для удобства
  typedef std::unique_ptr<response> ptr;
  typedef std::function< void(ptr) > handler;
};
```
   На вход генератора приходит ответ от предыдущего запроса, на основе которого он может создать следующий запрос. При первом запросе передается nullptr, а для того чтобы обозначить
   конец последовательности опроса генератор также возвращает nullptr. В примере простой случай, когда последовательность из одного запроса:
```cpp
request::ptr generator(response::ptr resp)
{
  if ( resp == nullptr )
    return std::make_unique<request>();
  return nullptr;
}
```
   Далее, сгенерированный запрос передается отправителю, который должен с ним что-то сделать и отправить ответ через callback-функцию в любой момент времени (как правило асинхронно).
   В примере просто формируем ответ через тот же workflow с задержкой:
```cpp
wflow::workflow wf(ios);
bool sender(request::ptr, response::handler callback)
{
  wf.post(std::chrono::milliseconds(500), [callback]()
  {
    callback(std::make_unique<response>());
  });
  return true;
}
```
   Отправитель возвращает true, аналогично обработчику таймера, сообщая что таймер должен продолжать работать. Создание опросника, аналогично созданию асинхронных таймеров:
```cpp
wf.create_requester<request, response>(std::chrono::seconds(1), sender, generator );
```
   В данном примере \ref example13.cpp генерируется последовательность из одного запроса с периодичностью в одну секунду. При более длинных последовательностях последующий запрос
   передается немедленно после получения ответа до тех пор пока генератор не вернет nullptr и только после этого наступает заданная пауза. Более сложный пример см. \ref example14.cpp
 */

class workflow
{
public:
  /// Переопределение boost::asio::io_context
  typedef boost::asio::io_context io_context_type;
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
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  /// Идентификатор таймера
  typedef int                               timer_id_t;

  typedef task_manager::timer_manager_t     timer_manager_t;

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
  explicit workflow(const workflow_options& opt, const workflow_handlers& handlers = workflow_handlers());

  /**
   * @brief Основной конструктор с поддержкой различных режимов.
   * @details Может работать как в основном потоке приложения, так и создавать
   * свой пул потоков. При workflow_options::threads == 0 дополнительные потоки не создаются о обработка
   * происходит при вызове io.run() или аналогичных. Для работы в многопоточном режиме
   * укажите количество потоков. При workflow_options::threads == 1 создается один поток и т.д. При многопоточном
   * режиме io не используется, а создается свой (один на все потоки этого объекта).
   * @remark При workflow_options::threads == 1 или большем создается свой объект io_context, а io не используется
   * @param io - ссылка на boost::asio::io_context.
   * @param opt - опции.
   */
  explicit workflow(io_context_type& io, const workflow_options& opt = workflow_options(),
                    const workflow_handlers& handlers = workflow_handlers() );

  /**
   * @brief Запуск потоков обработки.
   * @details Работает только в многопоточном режиме. При threads=0 ничего не происходит. Может
   * использоваться для возобновления работы после вызова workflow::stop.
   */
  void start();

  /**
   * @brief Реконфигурация после запуска.
   * @details Позволяет реконфигурировать workflow без сброса очередей.
   * Можно изменить число потоков или ограничения очереди.
   *
   * @attention Не работает при workflow_options::use_io_context == false
   * @param opt - новые опции.
   */
  bool reconfigure(const workflow_options& opt);
  bool reconfigure(const workflow_handlers& handlers);
  bool reconfigure(const workflow_options& opt, const workflow_handlers& handlers);

  /**
   * @brief Остановка потоков в многопоточном режиме со сбросом всех очередей.
   */
  void stop();

  /**
   * @brief Сброс всех очередей и таймеров.
   * @details Потоки обработки продолжат ожидание новых заданий.
   */
  void reset();

  /**
   * @brief Сброс всех таймеров и завершение работы потоков после обработки всех заданий
   * @details Позволяет завершить работу не сбрасывая очередь заданий. Отложенные
   * задания не сбрасываются, поэтому если в очереди есть задание с задержкой в сутки
   * то завершение работы произойдет через сутки. Для ожидания завершения заданий
   * используете метод workflow::wait. Убедитесь, что после вызова этого метода, не
   * происходит добавления заданий и создание таймеров, иначе останов может не произойти.
   * @ref example7.cpp
   */
  void shutdown();

  /**
   * @brief Ожидает завершения всех заданий после workflow::shutdown
   * @details Работает только в многопоточном режиме и имеет смысл только после вызова
   * метода workflow::shutdown. Блокирует выполнение программы до завершения работы потоков,
   * после обработки всех заданий в очереди. Убедитесь, что во время ожидания, не
   * происходит добавления заданий и создание таймеров. Если очередь заданий будет постоянно
   * пополняться то и время ожидания будет увеличиваться. Если во время ожидания создать
   * таймер, то и ожидание будет продолжаться, пока существует этот таймер.
   * @ref example7.cpp
   */
  void wait();

  /**
   * @brief Дождаться завершения всех заданий и продолжить работу
   * @details По сути реализует последовательность вызовов workflow::shutdown, workflow::wait и workflow::start.
   * */
  void wait_and_restart();

  /**
   * @brief Получить идентификатор workflow
   * @details Это произвольная строка, может использоваться при логгировании
   */
  std::string get_id() const;

  /**
   * @brief Получить текущие опции workflow
   */
  workflow_options get_options() const;

  /**
   * @brief Получить текущие обработчики workflow
   */
  workflow_handlers get_handlers() const;

  /**
   * @brief Отправить задание на обработку
   * @param handler обработчик задания типа void()
   * @param drop альтернативный обработчик, если задание выкидывается из очереди. По умолчанию nullptr (handler выкидывается из очереди без уведомления)
   * @return true - задание поставлено в очередь обработки. false - очередь переполнена, обработчик drop вызван
   *
   * При переполнении очереди drop может быть вызван как синхронно (чаще всего) так и асинхронно (если установлена опция rate_limit)
   */
  bool post(post_handler handler, drop_handler drop = nullptr);

  /**
   * @brief Отправить задание на обработку в указанный момент времени
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
   * @brief Отправить задание на обработку в указанный момент времени (строка в формате "22:00:00")
   * @param tp момент времени, в который задание будет отправлено на обработку
   * @param handler обработчик задания типа void()
   * @param drop альтернативный обработчик, если задание выкидывается из очереди. По умолчанию nullptr (handler выкидывается из очереди без уведомления)
   * @return true - включен таймер для отправки сообщения. Таймер выкинут из очереди быть не может поэтому всегда true
   *
   * В момент времени tp задание будет только отправлено в очередь на обработку (и может быть выкинуто, при переполнении ),
   * в отличии от safe_post, где оно будет выполнено сразу.
   */
  bool post(const std::string& tp, post_handler handler, drop_handler drop = nullptr);

  /**
   * @brief Отправить задание на обработку через указанный интервал времени
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
   * @brief Отправить задание на обработку игнорируя ограничения на размер очереди и скорости обработки.
   * @param handler обработчик задания типа void()
   *
   * Аналог boost::asio::io_context::post
   */
  void safe_post(post_handler handler);

  /**
   * @brief Выполнить задание в указанный момент времени
   * @param tp момент времени, в который задание будет выполнено
   * @param handler обработчик задания типа void()
   */
  void safe_post(time_point_t tp, post_handler handler);


  /**
   * @brief Выполнить задание в указанный момент времени (строка в формате "22:00:00")
   * @param tp момент времени, в который задание будет выполнено
   * @param handler обработчик задания типа void()
   */
  void safe_post(const std::string& tp, post_handler handler);

  /**
   * @brief Выполнить задание через указанный интервал времени
   * @param duration интервал времени, через который задание будет выполнено
   * @param handler обработчик задания типа void()
   */
  void safe_post(duration_t duration, post_handler handler);

  /**
   * @brief Создает таймер, обработчик которого выполняется с заданной периодичностью
   * @param duration интервал таймера
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(duration_t duration, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * @brief Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью
   * @param duration интервал таймера
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true),
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * (вызова callback_timer_handler из обработчика)
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * @brief Создает таймер, обработчик которого выполняется с заданной периодичностью через определенный интервал времени
   * @param start_duration интервал времени, через который запустить таймер
   * @param duration интервал таймера
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(duration_t start_duration, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * @brief Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью через определенный интервал времени
   * @param start_duration интервал времени, через который запустить таймер
   * @param duration интервал таймера
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true),
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * (вызова callback_timer_handler из обработчика)
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(duration_t start_duration, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * @brief Создает таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер
   * @param duration интервал таймера
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(time_point_t tp, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * @brief Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param tp момент времени, начиная с которого запустить таймер
   * @param duration интервал таймера
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true),
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * (вызова callback_timer_handler из обработчика)
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(time_point_t tp, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * @brief Создает таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param stp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param duration интервал таймера
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(std::string stp, duration_t duration, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * @brief Создает асинхронный таймер, обработчик которого выполняется с заданной периодичностью начиная с определенного момента времени
   * @param stp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param duration интервал таймера
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true),
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * (вызова callback_timer_handler из обработчика)
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(std::string stp, duration_t duration, async_timer_handler ahandler, expires_at expires = expires_at::after);


  /**
   * @brief Создает таймер, обработчик которого выполняется раз в сутки начиная с определенного момента времени
   * @param stp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param handler обработчик таймера std::function<bool()>, который должен возвращать true, если таймер должен продолжать работать
   * и false - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_timer(std::string stp, timer_handler handler, expires_at expires = expires_at::after);

  /**
   * @brief Создает асинхронный таймер, обработчик которого выполняется раз в сутки с определенного момента времени
   * @param stp момент времени, начиная с которого запустить таймер (строка в формате "22:00:00")
   * @param ahandler обработчик таймера std::function<void(callback_timer_handler)>, который должен вызвать callback_timer_handler(true),
   *           если таймер должен продолжать работать и callback_timer_handler(false) - для завершения.
   * @param expires если expires_at::after (по умолчанию), то отсчет до следующего запуска после выполнения обработчика таймера
   * (вызова callback_timer_handler из обработчика)
   *
   * @return идентификатор таймера, который можно использовать для остановки таймера
   */
  timer_id_t create_async_timer(std::string stp, async_timer_handler ahandler, expires_at expires = expires_at::after);

  /**
   * @brief Остановить таймер и освободить ресурсы
   * @param id идентификатор таймера
   * @return bool, если true - таймер остановлен, false - таймер с таким id не найдено
   */
  bool release_timer( timer_id_t id );

  /**
   * @brief Отсоединить таймер. Таймер продолжает работать до тех пор пока существует объект, который вернул detach_timer
   * @param id - идентификатор таймера
   * @return std::shared_ptr<bool>, если true - таймер работает , false - таймер с таким id не найдено
   *
   * простой вызов detach_timer эквивалентен release_timer, но если std::shared_ptr<bool> t = detach_timer(id), то таймер будет продолжать работать
   * до тех пор, пока t.reset() или t = nullptr
   */
  std::shared_ptr<bool> detach_timer(timer_id_t id);

  /** @brief Общее количество таймеров */
  size_t timer_count() const;

  /** @brief Общее количество обработчиков в очереди (включая таймеры) */
  size_t full_size() const;

  /** @brief Общее количество невыкидываемых обработчиков в очереди (включая таймеры) */
  size_t safe_size() const;

  /** @brief Количество обработчиков в очереди, которые могут быть выкинуты при переполнении */
  size_t unsafe_size() const;

  /** @brief Общее количество сброшенных обработчиков при переполнении */
  size_t dropped() const;

  /**
   * @brief Создает опросник с заданным интервалом
   * @tparam Req запрос
   * @tparam Res ответ
   * @param duration интервал опроса
   * @param sender отправитель запросов
   * @param generator генератор запросов
   *
   * ```cpp
   *
   * //// Типы sender и generator
   * typedef std::unique_ptr<Req> req_ptr;
   * typedef std::unique_ptr<Res> res_ptr;
   * typedef std::function<void(res_ptr)> callback;
   * typedef std::function< bool(req_ptr, callback) > sender_type;
   * typedef std::function< req_ptr(res_ptr) > sender_type;
   * ```
   * @see Концепция опросников
   */
  template< typename Req, typename Res>
  timer_id_t create_requester(
    duration_t duration,
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->get_timer_manager()->create<Req, Res>( duration, sender, generator);
  }

  /**
   * @brief Создает опросник, который начинает выполняться с заданной периодичностью через определенный интервал времени
   * @tparam Req запрос
   * @tparam Res ответ
   * @param start_duration интервал времени, через который запустить опросник
   * @param duration интервал опроса
   * @param sender отправитель запросов
   * @param generator генератор запросов
   *
   * ```cpp
   *
   * //// Типы sender и generator
   * typedef std::unique_ptr<Req> req_ptr;
   * typedef std::unique_ptr<Res> res_ptr;
   * typedef std::function<void(res_ptr)> callback;
   * typedef std::function< bool(req_ptr, callback) > sender_type;
   * typedef std::function< req_ptr(res_ptr) > sender_type;
   * ```
   * @see Концепция опросников
   */
  template< typename Req, typename Res>
  timer_id_t create_requester(
    duration_t start_duration,
    duration_t duration,
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->get_timer_manager()->create<Req, Res>( start_duration, duration, sender, generator );
  }

  /**
   * @brief Создает опросник, который начинает выполняться с заданной периодичностью с определенного момента времени
   * @tparam Req запрос
   * @tparam Res ответ
   * @param tp интервал времени, через который запустить опросник
   * @param duration интервал опроса
   * @param sender отправитель запросов
   * @param generator генератор запросов
   *
   * ```cpp
   *
   * //// Типы sender и generator
   * typedef std::unique_ptr<Req> req_ptr;
   * typedef std::unique_ptr<Res> res_ptr;
   * typedef std::function<void(res_ptr)> callback;
   * typedef std::function< bool(req_ptr, callback) > sender_type;
   * typedef std::function< req_ptr(res_ptr) > sender_type;
   * ```
   * @see Концепция опросников
   */
  template< typename Req, typename Res>
  timer_id_t create_requester(
    time_point_t tp,
    duration_t duration,
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->get_timer_manager()->create<Req, Res>( tp, duration, sender, generator );
  }

  /**
   * @brief Создает опросник с заданным интервалом
   * @tparam Req запрос
   * @tparam Res ответ
   * @param stp момент времени, начиная с которого запустить опросник (строка в формате "22:00:00")
   * @param duration интервал опроса
   * @param sender отправитель запросов
   * @param generator генератор запросов
   *
   * ```cpp
   *
   * //// Типы sender и generator
   * typedef std::unique_ptr<Req> req_ptr;
   * typedef std::unique_ptr<Res> res_ptr;
   * typedef std::function<void(res_ptr)> callback;
   * typedef std::function< bool(req_ptr, callback) > sender_type;
   * typedef std::function< req_ptr(res_ptr) > sender_type;
   * ```
   * @see Концепция опросников
   */
  template< typename Req, typename Res>
  timer_id_t create_requester(
    std::string stp,
    duration_t duration,
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->get_timer_manager()->create<Req, Res>(stp, duration, sender, generator);
  }

  /**
   * @brief Создает опросник, который начинает выполняться с заданной периодичностью с определенного момента времени
   * @tparam Req запрос
   * @tparam Res ответ
   * @param stp момент времени, начиная с которого запустить опросник (строка в формате "22:00:00")
   * @param sender отправитель запросов
   * @param generator генератор запросов
   *
   * ```cpp
   *
   * //// Типы sender и generator
   * typedef std::unique_ptr<Req> req_ptr;
   * typedef std::unique_ptr<Res> res_ptr;
   * typedef std::function<void(res_ptr)> callback;
   * typedef std::function< bool(req_ptr, callback) > sender_type;
   * typedef std::function< req_ptr(res_ptr) > sender_type;
   * ```
   * @see Концепция опросников
   */
  template< typename Req, typename Res>
  timer_id_t create_requester(
    std::string stp,
    typename requester::sender_t<Req, Res>::type sender,
    typename requester::generator_t<Req, Res>::type generator
  )
  {
    return this->get_timer_manager()->create<Req, Res>(stp, sender, generator);
  }

  std::shared_ptr<task_manager> get_task_manager() const;
  std::shared_ptr<timer_manager_t> get_timer_manager() const;
private:
  void initialize_();
  void create_wrn_timer_();
private:
  std::atomic<time_t> _delay_ms;
  std::shared_ptr<task_manager> _impl;

  typedef std::mutex mutex_type;
  mutable mutex_type _mutex;
  timer_id_t _wrn_timer = 0;
  workflow_options _opt;
  workflow_handlers _handlers;
};

}
