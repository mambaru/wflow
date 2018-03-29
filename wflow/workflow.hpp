#pragma once

#include <wflow/workflow_options.hpp>
#include <wflow/expires_at.hpp>
#include <wflow/task/task_manager.hpp>
#include <wflow/system/asio.hpp>
#include <chrono>

namespace wflow{

/**
 * @brief И это хорошо
 * @details подробное описание 
 */
class workflow
{
public:
  typedef ::wflow::asio::io_service io_service_type;
  typedef std::function< void() > post_handler;
  typedef std::function< void() > drop_handler;

  typedef std::function<bool()> timer_handler;
  typedef std::function<void(bool)> callback_timer_handler;
  typedef std::function<void(callback_timer_handler)> async_timer_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef time_point_t::duration            duration_t;
  typedef int                               timer_id_t;

  typedef task_manager::timer_type          timer_type;

public:
  virtual ~workflow();
  workflow(const workflow& ) = delete;
  workflow& operator=(const workflow& ) = delete;
  
  /**
   * @brief Конструктор без поддержки однопоточного режима.
   * @details В конфигурации необходимо указать threads=1 или больше 
   * При threads=0 задачи из очереди обрабатываться не будут
   * 
   * @param opt - опции. 
   */
  explicit workflow(workflow_options opt);
  
  /**
   * Универсальный конструктор с поддержкой нескольких потоков (при threads=1 или больше)
   * или однопоточный вариант (threads=0, в этом случае обработка очереди будет происходить 
   * при вызове io.run() или аналогичных. 
   * 
   * @param io - ссылка на boost::asio::io_service
   * 
   * При threads=1 или большем создается свой объект io_service, а io не используется 
   */
  workflow(io_service_type& io, workflow_options opt = workflow_options() );
  
  /**
   * Запуск потоков в многопоточном режиме. При threads=0 вызывать не обязательно
   */
  void start();
  
  /**
   * Реконфигурация после запуска. Можно изменить число потоков или ограничения очереди.
   * 
   * @param opt - новые опции. 
   */
  void reconfigure(workflow_options opt);
  
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
