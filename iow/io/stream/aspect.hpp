#pragma once


#include <iow/io/stream/ad_initialize.hpp>
#include <iow/io/stream/tags.hpp>

#include <iow/io/basic/tags.hpp>
#include <fas/aop.hpp>

/// include 
#include <iow/io/reader/stream/aspect.hpp>
#include <iow/io/writer/stream/aspect.hpp>

namespace iow{ namespace io{ namespace stream{
  

struct aspect: fas::aspect<
  typename ::iow::io::reader::stream::aspect::advice_list,
  typename ::iow::io::writer::stream::aspect::advice_list,
  fas::advice< _initialize_, ad_initialize>
>{};

}}}
