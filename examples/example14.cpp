#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <wflow/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
 * @example example14.cpp
 * @brief Более сложный пример requester'в.
 * @details В этом примере каждая вторая последовательность из десяти вызовов прерывается на пятом сообщении и начинается сначала.
 * После отработки пяти успешных последовательностей завершаем работу.
 */

/* Output:
create FIRST request
foo::method 0
create request N0.1
foo::method 1
create request N0.2
foo::method 2
create request N0.3
foo::method 3
create request N0.4
foo::method 4
create request N0.5
foo::method FAIL
create FIRST request
foo::method 0
create request N0.1
foo::method 1
create request N0.2
foo::method 2
create request N0.3
foo::method 3
create request N0.4
foo::method 4
create request N0.5
foo::method 5
create request N0.6
foo::method 6
create request N0.7
foo::method 7
create request N0.8
foo::method 8
create request N0.9
foo::method 9
STOP create request
 */

struct request
{
  int param = 0;
  // исключительно для удобства
  typedef std::unique_ptr<request> ptr;
};

struct response
{
  int result = 0;
  // исключительно для удобства
  typedef std::unique_ptr<response> ptr;
  typedef std::function< void(ptr) > handler;
};

class foo
{
public:
  explicit foo(wflow::workflow& w) noexcept
    : _workflow(w)
  {}

  void method( request::ptr req,  response::handler callback)
  {
    if ( req->param == 5 && _flag)
    {
      // Эмуляция ошибки на пятом запросе каждой второй последовательности
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
        response::ptr res(new response() );
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
  volatile int callcount = 0;
  wf.create_requester<request, response>(
    std::chrono::seconds(1),
    [f, &callcount](request::ptr req, response::handler callback)
    {
      // После отработки пяти успешных последовательностей завершаем работу.
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
        return request::ptr(new request() );
      }
      else if ( resp->result != 10)
      {
        std::cout << "create request N" << callcount << "." << resp->result << std::endl;
        request::ptr req(new request() );
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
