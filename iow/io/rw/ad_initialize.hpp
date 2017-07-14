#pragma once

#include <iow/io/rw/tags.hpp>
#include <iow/io/reader/data/tags.hpp>
#include <iow/io/writer/data/tags.hpp>
#include <iow/io/aux/global_pool.hpp>
namespace iow{ namespace io{ namespace rw{

struct ad_initialize
{
  template<typename T, typename O>
  void operator()(T& t, O opt)
  {
    opt.reader.create = global_pool::get_create();
    opt.reader.free = global_pool::get_free();
    opt.writer.create = global_pool::get_create();
    opt.writer.free = global_pool::get_free();
    t.get_aspect().template get< ::iow::io::reader::data::_initialize_ >()(t, opt.reader);
    t.get_aspect().template get< ::iow::io::writer::data::_initialize_ >()(t, opt.writer);
  }
};

}}}
