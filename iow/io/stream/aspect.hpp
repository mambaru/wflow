#pragma once


#include <iow/io/stream/ad_initialize.hpp>
#include <iow/io/stream/tags.hpp>

#include <iow/io/basic/tags.hpp>
#include <fas/aop.hpp>

/// include 
#include <iow/io/reader/data/aspect.hpp>
#include <iow/io/writer/data/aspect.hpp>

namespace iow{ namespace io{ namespace stream{
  

struct aspect: fas::aspect<
  typename ::iow::io::reader::data::aspect::advice_list,
  typename ::iow::io::writer::data::aspect::advice_list,
  fas::advice< _initialize_, ad_initialize>
>{};

}}}
