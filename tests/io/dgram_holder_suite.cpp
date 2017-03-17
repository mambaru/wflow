#include <iostream>
#include <iow/io/descriptor/holder.hpp>
#include <iow/io/socket/dgram/aspect.hpp>
#include <iow/io/socket/dgram/options.hpp>
#include <iow/io/socket/dgram/tags.hpp>
#include <iow/io/reader/asio/aspect.hpp>
#include <iow/io/writer/asio/aspect.hpp>
#include <iow/io/stream/aspect.hpp>
#include <iow/io/basic/aspect.hpp>

#include <fas/testing.hpp>

namespace {
typedef std::vector<char> data_type;
typedef iow::asio::posix::stream_descriptor descriptor1_type;
typedef iow::asio::ip::udp::socket descriptor2_type;
typedef iow::io::socket::dgram::options options_type;

struct aspect_stream : fas::aspect<
    fas::type< ::iow::io::descriptor::_descriptor_type_, descriptor2_type >,
    fas::type< ::iow::io::_options_type_, options_type >,
    ::iow::io::socket::dgram::aspect,
    fas::value< ::iow::io::socket::dgram::_current_endpoint_, std::shared_ptr< boost::asio::ip::udp::endpoint > >,
    ::iow::io::reader::asio::aspect,
    ::iow::io::writer::asio::aspect,
    ::iow::io::stream::aspect,
    ::iow::io::basic::aspect< std::recursive_mutex >::advice_list
>{};

typedef ::iow::io::descriptor::holder<aspect_stream> stream_holder;
}

UNIT(dgram_holder_unit, "")
{
  using namespace fas::testing;
  iow::asio::io_service service;
  boost::asio::ip::udp::endpoint ep(boost::asio::ip::udp::v4(), 12345);
  boost::asio::ip::udp::socket sock_server(service, ep );
  boost::asio::ip::udp::socket sock_client(service, boost::asio::ip::udp::v4());
  auto server = std::make_shared<stream_holder>( std::move(sock_server) );
  options_type opt;
  opt.reader.sep="";
  opt.writer.sep="";
  opt.incoming_handler = [&]( iow::io::data_ptr d, size_t, options_type::outgoing_handler_type)
  {
    server->get_aspect().get< ::iow::io::writer::_output_>()( *server, std::move(d) );
  };
  server->start(opt);
  sock_client.send_to( boost::asio::buffer("Привет Мир!!!"), ep );
  char result[1024]={'\0'};
  sock_client.async_receive_from(
    boost::asio::buffer(result, 1024),
    ep,
    [&result, &service](const boost::system::error_code& ec, std::size_t bytes_transferred)
    {
      result[bytes_transferred]='\0';
      service.stop();
    }
  );
  service.run();
  t << equal<assert, std::string>( result, "Привет Мир!!!" ) << FAS_FL;
}


BEGIN_SUITE(dgram_holder,"")
  ADD_UNIT(dgram_holder_unit)
END_SUITE(dgram_holder)

