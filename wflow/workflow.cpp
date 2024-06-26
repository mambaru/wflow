#include "logger.hpp"
#include "queue/bique.hpp"
#include "timer/private/time_parser.hpp"
#include "workflow.hpp"
namespace wflow{

workflow::~workflow()
{
  this->stop();
  _impl = nullptr;
}

workflow::workflow(const workflow_options& opt, const workflow_handlers& handlers )
  : _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(opt) )
  , _opt(opt)
  , _handlers(handlers)
{
  this->initialize_();
}

workflow::workflow(io_context_type& io, const workflow_options& opt, const workflow_handlers& handlers)
  : _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(io, opt) )
  , _opt(opt)
  , _handlers(handlers)
{
  this->initialize_();
}


void workflow::start()
{
  _impl->start();
}

bool workflow::reconfigure(const workflow_options& opt)
{
  return this->reconfigure(opt, this->get_handlers());
}

bool workflow::reconfigure(const workflow_handlers& handlers)
{
  return this->reconfigure(this->get_options(), handlers);
}

bool workflow::reconfigure(const workflow_options& opt, const workflow_handlers& handlers)
{
  if ( !_impl->reconfigure(opt) )
    return false;

  _delay_ms = opt.post_delay_ms;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _opt = opt;
    _handlers = handlers;
    this->initialize_();
  }
  return true;
}

std::string workflow::get_id() const
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _opt.id;
}

void workflow::reset()
{
  _impl->reset();
}

void workflow::stop()
{
  _owner.reset();
  _impl->stop();
}

void workflow::shutdown()
{
  _impl->shutdown();
}

void workflow::wait()
{
  _impl->wait();
}

void workflow::wait_and_restart()
{
  _impl->shutdown();
  _impl->wait();
  _impl->start();
}


std::shared_ptr< task_manager > workflow::get_task_manager() const
{
  return _impl;
}

std::shared_ptr< workflow::timer_manager_t> workflow::get_timer_manager() const
{
  return _impl->get_timer_manager();
}

workflow_options workflow::get_options() const
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _opt;
}

workflow_handlers workflow::get_handlers() const
{
  std::lock_guard<mutex_type> lk(_mutex);
  return _handlers;
}

void workflow::safe_post(post_handler handler)
{
  _impl->safe_post( handler );
}

bool workflow::post(post_handler handler, drop_handler drop)
{
  if ( _delay_ms == 0)
    return _impl->post( handler, drop );
  else
    return this->post( std::chrono::milliseconds(_delay_ms), handler, drop);
}

void workflow::safe_post(time_point_t tp, post_handler handler)
{
  _impl->safe_post_at( tp, handler );
}

bool workflow::post(time_point_t tp, post_handler handler, drop_handler drop)
{
  return _impl->post_at( tp, handler, drop);
}

void workflow::safe_post(const std::string& stp, post_handler handler)
{
  time_point_t tp;
  if ( time_parser::make_time_point(stp, &tp, nullptr) )
    this->safe_post( tp, handler );
  else
    this->safe_post( handler );
}

bool workflow::post(const std::string& stp, post_handler handler, drop_handler drop)
{
  time_point_t tp;
  if ( time_parser::make_time_point(stp, &tp, nullptr) )
    return this->post( tp, handler, drop );
  else
    return this->post( handler, drop );
}

void workflow::safe_post(duration_t d,   post_handler handler)
{
  return _impl->safe_delayed_post(d, handler);
}

bool workflow::post(duration_t d,   post_handler handler, drop_handler drop)
{
  return _impl->delayed_post(d, handler, drop);
}

workflow::timer_id_t workflow::create_timer(duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(duration_t sd, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create( sd, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(duration_t sd, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create( sd, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(time_point_t start_time, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(start_time, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(time_point_t start_time, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(start_time, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string start_time, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(start_time, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string start_time, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(start_time, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string schedule, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(schedule, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string schedule, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(schedule, handler, expires );
}

std::shared_ptr<bool> workflow::detach_timer(timer_id_t id)
{
  return _impl->get_timer_manager()->detach(id);
}

bool workflow::release_timer( timer_id_t id )
{
  return _impl->get_timer_manager()->release(id);
}

size_t workflow::timer_count() const
{
  return _impl->get_timer_manager()->size();
}

size_t workflow::full_size() const
{
  return _impl->full_size();
}

size_t workflow::safe_size() const
{
  return _impl->safe_size();
}

size_t workflow::unsafe_size() const
{
  return _impl->unsafe_size();
}

size_t workflow::dropped() const
{
  return _impl->dropped();
}

workflow::io_context_type& workflow::get_io_context()
{
  return _impl->get_io_context();  
}


void workflow::initialize_()
{
  _impl->rate_limit( _opt.rate_limit );
  _impl->set_startup( _handlers.startup_handler );
  _impl->set_status( _handlers.status_handler, _opt.status_ms );
  _impl->set_finish( _handlers.finish_handler );
  _impl->set_statistics( _handlers.statistics_handler );
  this->create_wrn_timer_();
}


void workflow::create_wrn_timer_()
{
  workflow& wrkf = _handlers.control_workflow == nullptr ? *this : *_handlers.control_workflow;

  auto old_timer = _wrn_timer;

  if ( _opt.control_ms!=0 )
  {
    auto dropsave = std::make_shared<size_t>(0);
    std::function<bool()> control_handler;

    std::string id = _opt.id;
    wlog::only_for_log(id);
    WFLOW_LOG_MESSAGE("Create overflow warning timer for '" << id << "'" );
    std::function<bool()> drop_wrn=[id](){WFLOW_LOG_WARNING("Workflow '" << id << "' drop warning timer" ); return false;};
    if (  _handlers.control_handler != nullptr )
    {
      control_handler = _handlers.control_handler;
    }
    else
    {
      size_t wrnsize = _opt.wrnsize;
      bool debug = _opt.debug;
      std::weak_ptr<task_manager> wtask = _impl;
      control_handler= [id, wtask, wrnsize, dropsave, debug]()  ->bool
      {
        if ( auto ptask = wtask.lock() )
        {
          auto dropcount = ptask->dropped();
          auto us_size = ptask->unsafe_size();
          auto s_size = ptask->safe_size();
          auto dropdiff = dropcount - *dropsave;
          wlog::only_for_log(s_size);
          if ( dropdiff!=0 )
          {
            WFLOW_LOG_ERROR("Workflow '" << id << "' queue dropped "
                            << dropdiff << " items (total " << dropcount << ", size "
                            << us_size << ", safe_size " << s_size <<  ")" )
            *dropsave = dropcount;
          }
          else if ( us_size > wrnsize )
          {
            WFLOW_LOG_WARNING("Workflow '" << id << "' queue size warning. Size " 
                              << us_size << " safe_size " << s_size << " (wrnsize=" << wrnsize << ")")
          }
          else if ( debug )
          {
            WFLOW_LOG_MESSAGE("Workflow '" << id << "' debug: total dropped " 
                              << dropcount << ", queue size=" << us_size << " + safe_size=" << s_size )
          }
          return true;
        }
        else
          return false;
      };
    };
    _wrn_timer = wrkf.create_timer(std::chrono::milliseconds(_opt.control_ms), 
                                   _owner.wrap( std::move(control_handler), std::move(drop_wrn) ) );
  }
  wrkf.release_timer(old_timer);
}

bool workflow::time_point_from_string(const std::string& strtime, time_point_t* tp, std::string* err)
{
  return time_parser::make_time_point(strtime, tp, err);
}

bool workflow::duration_from_string(const std::string& strtime, duration_t* tm, std::string* err)
{
  return time_parser::make_duration(strtime, tm, err);
}

}

/**
 * @example example01.cpp
 * @brief Простые примеры защищенных заданий в однопоточном режиме.
 * @remark Ожидание выполнения всех заданий с помощью io_context::run работает только в однопоточном режиме.
 */

/**
 * @example example02.cpp
 * @brief Простые примеры незащищенных заданий в однопоточном режиме.
 * @remark Ожидание выполнения всех заданий с помощью io_context::run работает только в однопоточном режиме.
 */

/**
 * @example example03.cpp
 * @brief Пример ограничения размера очереди.
 * @details В этом примере при превышении размера, задания не ставятся в очередь, а сразу вызывается drop-обработчик до запуска io_context::run().
 */

/**
 * @example example04.cpp
 * @brief Пример ограничения размера очереди для отложенных.
 * @details Для отложенных заданий, сначала создается таймер на постановку в очередь. Значение таймера номинальное, но сработает он только
 * после запуска io_context::run(), поэтому в отличии от \ref example03.cpp переполнение произойдет уже после запуска.
 */

/**
 * @example example05.cpp
 * @brief Еще пример таймера и различных заданий.
 */

/**
 * @example example06.cpp
 * @brief Пример многопоточной обработки.
 * @details В этом примере, в обработчике немного засыпаем, блокируя поток, чтобы подключились другие потоки и получить красивый вывод.
 */

/**
 * @example example07.cpp
 * @brief Пример многопоточной обработки с ограничением скорости обработки.
 * @details В этом примере ограничение на десять заданий в секунду. Первые десять заданий выполняются моментально,
 * а остальные более менее равномерно, но так, чтобы не превышать заданную скорость.
 */

/**
 * @example example08.cpp
 * @brief Пример динамической реконфигурации.
 * @details В этом примере ограничение по таймеру два раза реконфигурируем wflow::workflow, одновременно с этим, с отдельного потока,
 * максимально загружаем его пустыми заданиями.
 */

/**
 * @example example09.cpp
 * @brief Тестирование пропускной способности wflow::workflow с различным числом потоков на ограниченной по размеру очереди.
 * @details Число потоков передается параметром командной строки. Как видно из результатов, для легковесных заданий, наиболее
 * оптимальным является конфигурация с одним выделенным потоком или однопоточный вариант. Чтобы избежать потерь достаточно
 * увеличить размер очереди.
 * @remark Для отложенного на десять секунд задания для завершения работы не использовали отдельный wflow::workflow т.к. очередь
 * состоит исключительно из легковесных заданий и не бывает такого, что все потоки зависают на обработке без возможности отработать
 * таймер. Смотри /ref example11.cpp, где показана подобная задержка.
 */

/**
 * @example example10.cpp
 * @brief Тестирование пропускной способности wflow::workflow с различным числом потоков для защищенных заданий.
 * @details Число потоков передается параметром командной строки. Как видно из результатов, для легковесных заданий, наиболее
 * оптимальным является конфигурация с одним выделенным потоком или однопоточный вариант. Потерь здесь, как в /ref example09.cpp нет,
 * но и результаты немногим хуже.
 * @remark для отложенного на 10 секунд задания на завершения работы не использовали отдельный wflow::workflow т.к. очередь
 * состоит исключительно из легковесных заданий и не бывает такого, что все потоки зависают на обработке без возможности отработать
 * таймер. Смотри /ref example11.cpp, где показана подобная задержка.
 */

/**
 * @example example11.cpp
 * @brief Пример того, как можно забить очередь так, чтобы таймеры не сработали вовремя.
 * @details В конфигурации 4 потока отправляем 4 задания которые засыпают и не отпускают
 * поток на 4 секунды. Тут же отправляем отложенное на 2 секунды задание, но оно сработает
 * только через 4 секунды, когда освободится один из потоков.
 */

/**
 * @example example12.cpp
 * @brief Пример того, как решить проблему с задержкой таймера из примера /ref example11.cpp.
 * @details Для этого используем отдельный wflow::workflow для таймеров.
 */

/**
 * @example example13.cpp
 * @brief Простейший requester.
 * @details В этом примере последовательность вызовов состоит из одного запроса, который выполняется раз в секунду.
 * Для эмуляции асинхронного ответа используется тот же wflow::workflow.
 */

/**
 * @example example14.cpp
 * @brief Более сложный пример requester'в.
 * @details В этом примере каждая вторая последовательность из десяти вызовов прерывается на пятом сообщении и начинается сначала.
 * После отработки пяти успешных последовательностей завершаем работу.
 */

/**
 * @example example15.cpp
 * @brief Пример использования JSON-конфигурации
 * @details При запуске без параметров выводится сгенерированный JSON со значениями по умолчанию. В качестве параметра
 * передается JSON-конфигурация для wflow::workflow, через который данные из STDIN будут отправлены в STDOUT. Можно поэкспериментировать
 * с многопоточностью, задержками и ограничениями скорости обработки через конфиг.
 * ```bash
 * time cat ../examples/example15.cpp | ./examples/example15 ../examples/example15-1.json
 * ```
 */
