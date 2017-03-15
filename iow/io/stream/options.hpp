#pragma once

#include <iow/io/reader/stream/options.hpp>
#include <iow/io/writer/stream/options.hpp>

namespace iow{ namespace io{ namespace stream{

struct options
{
  typedef ::iow::io::reader::stream::options reader_options;
  typedef ::iow::io::writer::stream::options writer_options;
  reader_options reader;
  writer_options writer;
};

}}}
