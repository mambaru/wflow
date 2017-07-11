#pragma once

#include <iow/io/reader/data/options.hpp>
#include <iow/io/writer/data/options.hpp>
#include <iow/io/aux/data_pool_options.hpp>
#include <memory>

namespace iow{ namespace io{ namespace rw{

struct options
{
  typedef ::iow::io::reader::data::options reader_options;
  typedef ::iow::io::writer::data::options writer_options;
  reader_options reader;
  writer_options writer;
  /*data_map_options data_pool;*/
};

}}}
