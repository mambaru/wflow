#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

namespace {
struct request
{
  typedef std::unique_ptr<request> ptr;
  typedef std::function< void(ptr) > handler;
};

struct response
{
  typedef std::unique_ptr<response> ptr;
  typedef std::function< void(ptr) > handler;
};

inline bool sender(request::ptr, response::handler callback);
inline request::ptr generator(response::ptr resp);

boost::asio::io_service ios;
wflow::workflow wf(ios);

bool sender(request::ptr, response::handler callback)
{
  wf.post(std::chrono::milliseconds(500), [callback]()
  {
    std::cout << "response ready" << std::endl;
    callback(std::make_unique<response>());
  });
  return true;
}

request::ptr generator(response::ptr resp)
{
  if ( resp == nullptr )
  {
    std::cout << "generate first" << std::endl;
    return std::make_unique<request>();
  }
  std::cout << "generate finish flag" << std::endl;
  return nullptr;
}

} 

int main()
{
  wf.create_requester<request, response>(std::chrono::seconds(1), sender, generator );
  ios.run();
}
