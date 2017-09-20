#pragma once

namespace iow{ namespace io{ namespace descriptor{

template<typename ContextType>
struct options
{
private:
  typedef ContextType context_type;
public:
  typedef typename context_type::output_handler_type  output_handler_type;
  typedef typename context_type::input_handler_type  input_handler_type;
  typedef typename context_type::startup_handler_type   startup_handler_type;
  typedef typename context_type::shutdown_handler_type  shutdown_handler_type;
  typedef typename context_type::fatal_handler_type     fatal_handler_type;

  bool nonblocking = true; 
  size_t receive_buffer_size = 0;
  size_t send_buffer_size = 0;
  output_handler_type output_handler;
  input_handler_type input_handler;
  startup_handler_type  startup_handler;
  shutdown_handler_type shutdown_handler;
  fatal_handler_type    fatal_handler;
 
};
  
}}}
