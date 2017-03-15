#pragma once

#include <iow/io/reader/data/ad_clear.hpp>
#include <iow/io/reader/data/ad_initialize.hpp>
#include <iow/io/reader/data/ad_confirm.hpp>
#include <iow/io/reader/data/ad_rollback.hpp>
#include <iow/io/reader/data/ad_handler.hpp>
#include <iow/io/reader/data/ad_next.hpp>
#include <iow/io/reader/data/tags.hpp>

#include <iow/io/reader/aspect.hpp>
#include <iow/io/aux/read_buffer.hpp>
#include <iow/io/basic/tags.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace io{ namespace reader{ namespace data{
  
struct aspect: fas::aspect<
  ::iow::io::reader::aspect::advice_list,
  fas::advice< _initialize_, ad_initialize>,
  fas::advice< _clear_, ad_clear>,
  fas::group< ::iow::io::_after_stop_, _clear_>,
  fas::group< ::iow::io::_after_reset_, _clear_>,
  fas::advice< ::iow::io::reader::_next_, ad_next >,
  fas::advice< ::iow::io::reader::_confirm_, ad_confirm>,
  fas::advice< ::iow::io::reader::_rollback_, ad_rollback>,
  fas::advice< ::iow::io::reader::_handler_,  ad_handler>,
  fas::value< _read_buffer_, ::iow::io::read_buffer >
>{};

}}}}
