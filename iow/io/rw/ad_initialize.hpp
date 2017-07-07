#pragma once

#include <iow/io/rw/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <iow/io/aux/global_pool.hpp>
namespace iow{ namespace io{ namespace rw{

  /*
typedef data_pool< data_type > data_pool_t;
struct data_pool_impl: data_pool_t {};
typedef std::shared_ptr< data_pool_impl > data_pool_ptr;
*/
struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O opt)
  {
    opt.reader.create = global_pool::get_create();
    opt.reader.free = global_pool::get_free();
    opt.writer.create = global_pool::get_create();
    opt.writer.free = global_pool::get_free();
    /*
    if ( false == opt.data_pool.disabled)
    {
      auto pool = std::make_shared< data_pool_impl >();
      pool->set_options(opt.data_pool);
      IOW_LOG_MESSAGE("Initialize data pool with poolsize " << opt.data_pool.poolsize << " buffers")
      t.get_aspect().template get< _buffer_pool_ >() = pool;
      using namespace std::placeholders;
      opt.reader.create = std::bind( &data_pool_t::create, pool, _1, _2);
      opt.reader.free = std::bind(&data_pool_t::free, pool, _1);
      opt.writer.create = std::bind( &data_pool_t::create, pool, _1, _2);
      opt.writer.free = std::bind(&data_pool_t::free, pool, _1);
    }
    */
    
    t.get_aspect().template get< ::iow::io::reader::data::_initialize_ >()(t, opt.reader);
    t.get_aspect().template get< ::iow::io::writer::data::_initialize_ >()(t, opt.writer);
  }
};

}}}
