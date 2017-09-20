#pragma once

#include <memory>

#include <fas/aop.hpp>
#include <fas/integral/bool_.hpp>
#include <iow/logger/logger.hpp>
#include <iow/asio.hpp>
#include <thread>

namespace iow{ namespace io{ namespace descriptor{ 

// Универсальный для всех
template<typename TgInitialize, bool MakeOutgoingHandler>
struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O&& opt)  const
  {
    typedef typename T::aspect::template advice_cast< _context_>::type context_type;
    context_type& cntx = t.get_aspect().template get<_context_>();

    cntx.output_handler  = opt.output_handler;
    cntx.input_handler  = opt.input_handler;
    cntx.startup_handler   = opt.startup_handler;
    cntx.shutdown_handler  = opt.shutdown_handler;
    cntx.fatal_handler     = opt.fatal_handler;

    if (  opt.input_handler != nullptr )
    {
      this->make_output_(t, cntx, fas::bool_<MakeOutgoingHandler>() );
    }

    if (  cntx.fatal_handler == nullptr )
    {
      cntx.fatal_handler = [](int code, std::string message)
      {
        IOW_LOG_FATAL("iow::io::descriptor: Default fatal handler (" << code << ") " << message)
      };
    }

    iow::asio::socket_base::non_blocking_io non_blocking_io(opt.nonblocking);
    t.descriptor().io_control(non_blocking_io);

    if ( opt.receive_buffer_size != 0 )
    {
      boost::asio::socket_base::receive_buffer_size option(opt.receive_buffer_size);
      t.descriptor().set_option(option);
    }

    if ( opt.send_buffer_size != 0 )
    {
      boost::asio::socket_base::send_buffer_size option(opt.send_buffer_size);
      t.descriptor().set_option(option);
    }

    t.get_aspect().template get< TgInitialize >()( t, std::forward<O>(opt) );
  }

private:

  template<typename T, typename Cntx> 
  static void make_output_(T& t, Cntx& cntx, fas::true_ )
  {
    io_id_t io_id = t.get_id_(t);
    typedef Cntx context_type;
    auto callback = cntx.output_handler;
    auto input = cntx.input_handler;
    std::weak_ptr<T> wthis = t.shared_from_this();
    cntx.output_handler = t.wrap([wthis, callback](typename context_type::data_ptr d)
    {
      if ( auto pthis = wthis.lock() )
      {
        if ( callback != nullptr )
        {
          callback(std::move(d));
        }
        else
        {
          bool shutdown = (d == nullptr );
          std::lock_guard<typename T::mutex_type> lk( pthis->mutex() );
          pthis->get_aspect().template get<_output_>()( *pthis, std::move(d) );
          if ( shutdown )
            pthis->get_aspect().template get< ::iow::io::_shutdown_>()( *pthis, nullptr );
        }
      }
    }, 
    [wthis, io_id, input](typename context_type::data_ptr d) ->void
    { 
      if ( d != nullptr )
      {
      }
      
      if ( input ) 
      {
        if ( auto pthis = wthis.lock() )
        {
          input(nullptr, io_id, nullptr);
        }
      }
      else
      {
        if ( auto pthis = wthis.lock() )
        {
          pthis->get_aspect().template get<_input_>()( *pthis, nullptr );
        }
      }
    }
    );
  }

  template<typename T, typename Cntx>
  static void make_output_(T& /*t*/, Cntx& /*cntx*/, fas::false_ )
  {
  }

};

  
}}}
