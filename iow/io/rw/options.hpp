#pragma once

#include <iow/io/reader/data/options.hpp>
#include <iow/io/writer/data/options.hpp>
#include <memory>

namespace iow{ namespace io{ namespace rw{

struct options
{
  typedef ::iow::io::reader::data::options reader_options;
  typedef ::iow::io::writer::data::options writer_options;
  reader_options reader;
  writer_options writer;

};

}}}
