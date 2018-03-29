#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

struct request{ int param = 0; };
struct response{ int result = 0; };

/*struct ifoo
{
  virtual void method( std::unique_ptr<request>,  std::function< void(std::unique_ptr<response>) > ) = 0;
};*/

class foo/*: public ifoo*/
{
public:
  foo(wflow::workflow& w)
    : _workflow(w)
  {}
  
  void method( std::unique_ptr<request> req,  std::function< void(std::unique_ptr<response>) > callback) 
  {
    if ( req->param == 5 && _flag)
    {
      std::cout << "foo::method FAIL" << std::endl;
      _flag = false;
      callback(nullptr);
      return;
    }
    
    if ( req->param == 5 )
      _flag = true;
    
    auto preq = std::make_shared<request>( std::move(*req) );
    _workflow.post(
      std::chrono::milliseconds(100),
      [preq, callback]()
      {
        std::cout << "foo::method " << preq->param << std::endl;
        auto res = std::make_unique<response>();
        res->result = preq->param + 1;
        callback( std::move(res) );
      }
    );
  }
  
private:
  wflow::workflow& _workflow;
  bool _flag = true;
};

int main()
{
  boost::asio::io_service ios;
  wflow::workflow wf(ios);
  auto f = std::make_shared<foo>(wf);
  
  wf.create_requester<request, response>(
    std::chrono::seconds(1),
    f, 
    &foo::method,
    [](std::unique_ptr<response> resp) -> std::unique_ptr<request>
    {
      if ( resp == nullptr )
      {
        std::cout << "create FIRST request "  << std::endl;
        return std::make_unique<request>();
      }
      else if ( resp->result != 10)
      {
        std::cout << "create request N" << resp->result << std::endl;
        auto req = std::make_unique<request>();
        req->param = resp->result;
        return req;
      }
      else
      {
        std::cout << "STOP create request" << std::endl;
        return nullptr;
      }
    }
  );
  ios.run();
}
