#include "logger.hpp"
#include <wflow/queue/bique.hpp>
#include "workflow.hpp"

namespace wflow{
  

workflow::~workflow()
{
  this->stop();
  _impl = nullptr;
}

workflow::workflow(workflow_options opt )
  : _id( opt.id )
  , _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(opt.maxsize, opt.threads, opt.use_io_service) )
  , _workflow_ptr(opt.control_workflow)
{
  this->initialize_(opt);
}

workflow::workflow(io_service_type& io, workflow_options opt)
  : _id( opt.id )
  , _delay_ms(opt.post_delay_ms)
  , _impl( std::make_shared<task_manager>(io, opt.maxsize, opt.threads, opt.use_io_service) )
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

void workflow::reconfigure(workflow_options opt)
{
  _id = opt.id;
  _workflow_ptr = opt.control_workflow;
  _impl->rate_limit( opt.rate_limit );
  _impl->set_startup( opt.startup_handler );
  _impl->set_finish( opt.finish_handler );
  _impl->set_statistics( opt.statistics_handler );
  _impl->reconfigure(opt.maxsize, opt.threads, opt.use_io_service);
  _delay_ms = opt.post_delay_ms;
  this->create_wrn_timer_(opt);
}

void workflow::clear()
{
  _impl->reset();
}

void workflow::stop()
{
  _impl->stop();
}

std::shared_ptr< task_manager > workflow::manager() const
{
  return _impl;
}

std::shared_ptr< workflow::timer_type> workflow::get_timer() const
{
  return _impl->timer();
}

void workflow::safe_post(post_handler handler)
{
  if ( _delay_ms == 0)
    _impl->safe_post( handler );
  else
    this->safe_post( std::chrono::milliseconds(_delay_ms), handler );
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
  return _impl->timer()->create(d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(duration_t sd, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->timer()->create( sd, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(duration_t sd, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->timer()->create( sd, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(time_point_t tp, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(time_point_t tp, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string tp, duration_t d, timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string tp, duration_t d, async_timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, d, handler, expires );
}

workflow::timer_id_t workflow::create_timer(std::string tp, timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, handler, expires );
}

workflow::timer_id_t workflow::create_async_timer(std::string tp, async_timer_handler handler, expires_at expires)
{
  return _impl->timer()->create(tp, handler, expires );
}


std::shared_ptr<bool> workflow::detach_timer(timer_id_t id)
{
  return _impl->timer()->detach(id);
}

bool workflow::release_timer( timer_id_t id )
{
  return _impl->timer()->release(id);
}
  
size_t workflow::timer_count() const
{
  return _impl->timer()->size();
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
  workflow& wrkf = _workflow_ptr == 0 ? *this : *_workflow_ptr;
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
        auto dropped = this->_impl->dropped();
        auto size = this->_impl->full_size();
        auto dropdiff = dropped - *dropsave;
        if ( dropdiff!=0 )
        {
          WFLOW_LOG_ERROR("Workflow '" << this->_id << "' queue dropped " << dropdiff << " items (total " << dropped << ", size " << size << ")" )
          *dropsave = dropped;
        }
        else if ( size > wrnsize )
        {
          WFLOW_LOG_WARNING("Workflow '" << this->_id << "' queue size warning. Size " << size << " (wrnsize=" << wrnsize << ")")
        } 
        else if ( debug )
        {
          WFLOW_LOG_MESSAGE("Workflow '" << this->_id << "' debug: total dropped " << dropped << ", queue size " << size)
        }
        return true;
      };
    }
    _wrn_timer = wrkf.create_timer(std::chrono::milliseconds(opt.control_ms), control_handler );
  }
  wrkf.release_timer(old_timer);
}
  
}
