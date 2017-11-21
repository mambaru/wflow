#pragma once

#include <iow/io/descriptor/tags.hpp>
#include <iow/logger/logger.hpp>
#include <memory>
#include <mutex>
namespace iow{ namespace io{ namespace descriptor{

struct ad_input_handler
{
  template<typename T, typename D>
  void operator()(T& t, D d) const
  {
    const auto& cntx = t.get_aspect().template get<_context_>();

    if ( cntx.input_handler != nullptr )
    {
      /*
      auto input = cntx.input_handler;
      auto output = t.get_aspect().template get<_make_output_>()(t);
      auto io_id = t.get_id_(t);
      auto& m = t.mutex();
      m.unlock();
      input( std::move(d), std::move(io_id), std::move(output));
      m.lock();
      */
      
      
      auto input = cntx.input_handler;
      //auto output = cntx.output_handler;
      auto output = t.get_aspect().template get<_make_output_>()(t);
      auto io_id = t.get_id_(t);
      t.mutex().unlock();
      try
      {
        input( std::move(d), std::move(io_id), std::move(output));
      }
      catch(const std::exception& e)
      {
        if ( output!=nullptr )
          output(nullptr);

        std::lock_guard<typename T::mutex_type> lk(t.mutex());
        if ( cntx.fatal_handler != nullptr ) 
          try 
        {
          cntx.fatal_handler(-1, std::string("iow::io::descriptor::ad_input_handler: std::exception: ") + std::string(e.what()));
        } 
        catch(...) {}
      }
      catch(...)
      {
        if ( output ) output(nullptr);
        std::lock_guard<typename T::mutex_type> lk(t.mutex());
        if ( cntx.fatal_handler != nullptr ) try {
          cntx.fatal_handler(-1, "iow::io::descriptor::ad_input_handler: Unhandled exception in input handler");
          } catch(...) {}
      }
      t.mutex().lock();
      
    }
    else
    {
      t.get_aspect().template get<_output_>()( t, std::move(d) );
    }
  }
};
  
}}}
