#include <wflow/workflow.hpp>
#include <wflow/logger.hpp>
#include <fas/system/memory.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <memory>

/**
 * @example example13.cpp
 * @brief Простейший requester.
 * @details В этом примере последовательность вызовов состоит из одного запроса, который выполняется раз в секунду.
 * Для эмуляции асинхронного ответа используется тот же wflow::workflow.
 */

/*
Output:
  generate first
  response ready
  finish chain
  generate first
  response ready
  finish chain
  ...
*/


namespace {

boost::asio::io_context ios;
wflow::workflow wf(ios);

// Произвольный тип запроса
struct request
{
  // исключительно для удобства
  typedef std::unique_ptr<request> ptr;
  typedef std::function< void(ptr) > handler;
};

// Произвольный тип ответа
struct response
{
  // исключительно для удобства
  typedef std::unique_ptr<response> ptr;
  typedef std::function< void(ptr) > handler;
};

// Отправитель. Должен обработать (асинхронно) запрос, а ответ отправить через callback
inline bool sender(request::ptr, response::handler callback);

// Генератор по ответу на предедущий запрос создает новый запрос
inline request::ptr generator(response::ptr resp);

// в данном случае эмулируем асинхронное взаимодествие wflow::workflow
bool sender(request::ptr, response::handler callback)
{
  wf.post(std::chrono::milliseconds(500), [callback]()
  {
    std::cout << "response ready" << std::endl;
    // Отправляем ответ через 500 мс
    callback( response::ptr(new response() ));
  });
  return true;
}

request::ptr generator(response::ptr resp)
{
  // Значение resp==nullptr говорит о том, что это начало последовательности вызовов
  if ( resp == nullptr )
  {
    std::cout << "generate first" << std::endl;
    return request::ptr(new request() );
  }

  // Так как последовательность из одного вызова, то на resp!=nullptr возвращаем nullptr
  // cообщая, что последовательность завершена
  std::cout << "finish chain" << std::endl;
  return nullptr;
}

}

int main()
{
  wf.create_requester<request, response>(std::chrono::seconds(1), sender, generator );
  ios.run();
}
