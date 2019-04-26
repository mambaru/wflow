#include "logger.hpp"
#include <wflow/queue/bique.hpp>
#include "workflow.hpp"

namespace wflow{
  
workflow::~workflow()
{
  this->stop();
  _impl = nullptr;
}

workflow::workflow(const workflow_options& opt )
  : _id( opt.id )
  , _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(opt) )
  , _workflow_ptr(opt.control_workflow)
{
  this->initialize_(opt);
}

workflow::workflow(io_service_type& io, const workflow_options& opt)
  : _id( opt.id )
  , _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(io, opt) )
  , _workflow_ptr(opt.control_workflow)
{
  this->initialize_(opt);
}

void workflow::initialize_(const workflow_options& opt)
{
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  this->create_wrn_timer_(opt);
}

void workflow::start()
{
  _impl->start();
}

bool workflow::reconfigure(const workflow_options& opt)
{
  if ( !_impl->reconfigure(opt) )
    return false;
  _id = opt.id;
  _workflow_ptr = opt.control_workflow;
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  _delay_ms = opt.post_delay_ms;
  this->create_wrn_timer_(opt);
  return true;
}

const std::string& workflow::get_id() const
{
  return _id;
}

void workflow::reset()
{
  _impl->reset();
}

void workflow::stop()
{
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


std::shared_ptr< task_manager > workflow::get_task_manager() const
{
  return _impl;
}

std::shared_ptr< workflow::timer_manager_t> workflow::get_timer_manager() const
{
  return _impl->get_timer_manager();
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

workflow::timer_id_t workflow::create_timer(time_point_t tp, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(time_point_t tp, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string tp, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string tp, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string tp, timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string tp, async_timer_handler handler, expires_at expires)
{
  return _impl->get_timer_manager()->create(tp, handler, expires );
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

void workflow::create_wrn_timer_(const workflow_options& opt)
{
  workflow& wrkf = _workflow_ptr == nullptr ? *this : *_workflow_ptr;
  
  auto old_timer = _wrn_timer;
  
  if ( opt.control_ms!=0 )
  {
    auto dropsave = std::make_shared<size_t>(0);
    std::function<bool()> control_handler;
    if (  opt.control_handler != nullptr )
    {
      control_handler = opt.control_handler;
    }
    else
    {
      size_t wrnsize = opt.wrnsize;
      bool debug = opt.debug;
      control_handler= [this, wrnsize, dropsave, debug]()  ->bool 
      {
        auto dropcount = this->_impl->dropped();
        auto us_size = this->_impl->unsafe_size();
        auto s_size = this->_impl->safe_size();
        auto dropdiff = dropcount - *dropsave;
        wlog::only_for_log(s_size);
        if ( dropdiff!=0 )
        {
          WFLOW_LOG_ERROR("Workflow '" << this->_id << "' queue dropped " 
                          << dropdiff << " items (total " << dropcount << ", size " 
                          << us_size << ", safe_size " << s_size <<  ")" )
          *dropsave = dropcount;
        }
        else if ( us_size > wrnsize )
        {
          WFLOW_LOG_WARNING("Workflow '" << this->_id << "' queue size warning. Size " << us_size << " safe_size " << s_size << " (wrnsize=" << wrnsize << ")")
        } 
        else if ( debug )
        {
          WFLOW_LOG_MESSAGE("Workflow '" << this->_id << "' debug: total dropped " << dropcount << ", queue size=" << us_size << " + safe_size=" << s_size )
        }
        return true;
      };
    }
    _wrn_timer = wrkf.create_timer(std::chrono::milliseconds(opt.control_ms), control_handler );
  }
  wrkf.release_timer(old_timer);
}

}

/**
 * @example example01.cpp
 * @brief Простые примеры защищенных заданий в однопоточном режиме.
 * @remark Ожидание выполнения всех заданий с помощью io_service::run работает только в однопоточном режиме.
 */

/**
 * @example example02.cpp
 * @brief Простые примеры незащищенных заданий в однопоточном режиме.
 * @remark Ожидание выполнения всех заданий с помощью io_service::run работает только в однопоточном режиме.
 */

/**
 * @example example03.cpp
 * @brief Пример ограничения размера очереди.
 * @details В этом примере при превышении размера, задания не ставятся в очередь, а сразу вызывается drop-обработчик до запуска io_service::run().
 */

/**
 * @example example04.cpp
 * @brief Пример ограничения размера очереди для отложенных.
 * @details Для отложенных заданий, сначала создается таймер на постановку в очередь. Значение таймера номинальное, но сработает он только 
 * после запуска io_service::run(), поэтому в отличии от \ref example03.cpp переполнение произойдет уже после запуска.
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
