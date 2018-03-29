#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

struct request{ };
struct response{ };

class foo
{
public:
  void method( std::unique_ptr<request>,  std::function< void(std::unique_ptr<response>) > callback) 
  {
    std::cout << "foo::method" << std::endl;
    callback( std::make_unique<response>() );
  }
};

int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  auto f = std::make_shared<foo>();
  
  wf.create_requester<request, response>(
    std::chrono::seconds(1),
    [f](std::unique_ptr<request> req, std::function< void(std::unique_ptr<response>) > callback)
    {
      f->method(std::move(req), callback);
      return true;
    },
    [](std::unique_ptr<response> resp) -> std::unique_ptr<request>
    {
      
      if ( resp == nullptr )
        return std::make_unique<request>();
      
      return nullptr;
    }
  );
  ios.run();
}
