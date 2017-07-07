#pragma once


#include <iow/io/rw/ad_initialize.hpp>
#include <iow/io/rw/tags.hpp>

#include <iow/io/basic/tags.hpp>
#include <fas/aop.hpp>

#include <iow/io/aux/data_pool.hpp>
#include <iow/io/reader/data/aspect.hpp>
#include <iow/io/writer/data/aspect.hpp>

namespace iow{ namespace io{ namespace rw{


struct aspect: fas::aspect<
  typename ::iow::io::reader::data::aspect::advice_list,
  typename ::iow::io::writer::data::aspect::advice_list,
  fas::advice< _initialize_, ad_initialize>/*,
  fas::value< _buffer_pool_, data_pool_ptr >*/
>{};

}}}
