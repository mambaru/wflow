#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

struct request
{
  int param = 0; 
  /* исключительно для удобства*/
  typedef std::unique_ptr<request> ptr;
};

struct response
{
  int result = 0; 
  /* исключительно для удобства*/
  typedef std::unique_ptr<response> ptr;
  typedef std::function< void(ptr) > handler;
};

class foo
{
public:
  foo(wflow::workflow& w)
    : _workflow(w)
  {}
  
  void method( request::ptr req,  response::handler callback) 
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
  int callcount = 0;
  wf.create_requester<request, response>(
    std::chrono::seconds(1),
    [f, &callcount](request::ptr req, response::handler callback)
    {
      if ( callcount == 5 )
        return false;
      f->method(std::move(req), callback);
      return true;
    },
    [&callcount](response::ptr resp) -> request::ptr
    {
      if ( resp == nullptr )
      {
        std::cout << "create FIRST request "  << std::endl;
        return std::make_unique<request>();
      }
      else if ( resp->result != 10)
      {
        std::cout << "create request N" << callcount << "." << resp->result << std::endl;
        auto req = std::make_unique<request>();
        req->param = resp->result;
        return req;
      }
      else
      {
        std::cout << "STOP create request" << std::endl;
        ++callcount;
        return nullptr;
      }
    }
  );
  ios.run();
}
