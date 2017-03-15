#pragma once

#include <iow/io/reader/data/options.hpp>
#include <iow/io/writer/data/options.hpp>

namespace iow{ namespace io{ namespace stream{

struct options
{
  typedef ::iow::io::reader::data::options reader_options;
  typedef ::iow::io::writer::data::options writer_options;
  reader_options reader;
  writer_options writer;
};

}}}
