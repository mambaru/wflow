#include <wflow/workflow.hpp>
#include <wflow/owner.hpp>
#include <iostream>
#include <chrono>
#include <functional>

/**
 * @example example20.cpp
 * @brief Owner::callback: один вызов, double-call и no-call.
 * @details Три сценария ответа «с той стороны»:
 *   1) колбэк вызвали ровно один раз — норма;
 *   2) вызвали дважды — основной код один раз, срабатывает double_call_handler;
 *   3) колбэк уничтожили, так и не вызвав — срабатывает no_call_handler.
 */

/* Output:
  ok: result=42
  should print once
  double-call detected
  no-call: callback destroyed without invoke
*/

int main()
{
  wflow::owner own;
  own.set_double_call_handler([](){
    std::cout << "double-call detected" << std::endl;
  });
  own.set_no_call_handler([](){
    std::cout << "no-call: callback destroyed without invoke" << std::endl;
  });

  // 1) Нормальный ответ ровно один раз
  {
    auto cb = own.callback([](int v){
      std::cout << "ok: result=" << v << std::endl;
    });
    cb(42);
  }

  // 2) Ответили дважды — второй раз в основной handler не идём
  {
    auto cb = own.callback([](int){
      std::cout << "should print once" << std::endl;
    });
    cb(1);
    cb(2); // double-call
  }

  // 3) Колбэк ушёл из области видимости без вызова
  {
    auto cb = own.callback([](int){
      std::cout << "never" << std::endl;
    });
    (void)cb;
  } // ~callback_control → no_call

  // Небольшая пауза не нужна: всё синхронно. Для единообразия с workflow-примерами
  // можно было бы увести вызовы в очередь — смысл callback от этого не меняется.
}
