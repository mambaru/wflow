
[![tag](https://img.shields.io/github/v/tag/mambaru/wflow.svg?sort=semver)](https://github.com/mambaru/wflow/tree/master)
[![Build Status](https://github.com/mambaru/wflow/workflows/C++%20CI/badge.svg?branch=master)](https://github.com/mambaru/wflow/tree/master)
[![Build Status](https://github.com/mambaru/wflow/workflows/C++%20CI/badge.svg?branch=mambaru)](https://github.com/mambaru/wflow/tree/mambaru)
[![Build Status](https://travis-ci.com/mambaru/wflow.svg?branch=master)](https://travis-ci.com/mambaru/wflow)
[![Build Status](https://travis-ci.com/mambaru/wflow.svg?branch=mambaru)](https://travis-ci.com/mambaru/wflow)
[![codecov](https://codecov.io/gh/mambaru/wflow/branch/master/graph/badge.svg)](https://codecov.io/gh/mambaru/wflow)

Библиотека на базе boost::asio для работы с потоками и очередями, с возможностью динамического реконфигурирования и удобными таймерами.

* Документация [doxygen](https://mambaru.github.io/wflow/index.html).
* Репозитарий на [github.com](https://github.com/mambaru/wflow).
* Отчет [coverage](https://mambaru.github.io/wflow/cov-report/index.html)

# Сборка и установка

```bash
Usage:
        make help
        make shared
        make static
        make tests
```
Если установлен doxygen то в папке `/docs/html` будет документация.

Для сборки примеров и тестов, а также чтобы отключить поддержку JSON-конфигурации и/или логирования :

```bash
git clone https://github.com/mambaru/wflow.git
mkdir wflow/build
cd wflow/build
cmake ..
# Для сборки примеров и тестов
cmake -DBUILD_TESTING=ON ..
# Если поддержка JSON-конфигурации не требуется
#   cmake -DWFLOW_DISABLE_JSON=ON -DWLOG_DISABLE_JSON=ON ..
# Если поддержка логирования не требуется
#   cmake -DWFLOW_DISABLE_LOG=ON ..
cmake --build make
ctest
```
Для компиляции с поддержкой JSON-конфигурации потребуются библиотеки faslib, wjson, wlog, которые система сборки автоматически клонирует
в директорию проекта, если не найдет их в системе.

# Некоторые примеры

Все примеры с описанием в `examples`.

Гарантированное выполнение заданий:
```cpp
int main()
{
  boost::asio::io_context ios;
  wflow::workflow wf(ios);

  // Простое задание
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );

  // Отложенное задание
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );

  // Задание на конкретный момент времени
  auto tp = std::chrono::system_clock::now();
  tp += std::chrono::seconds(2);
  wf.safe_post( tp, [](){ std::cout << "Safe post in time point" << std::endl; } );

  // Ожидаем выполнение всех заданий
  ios.run();
}
```

Пример ограничения размера очереди:
```cpp
int main()
{
  boost::asio::io_context ios;
  wflow::workflow_options opt;
  opt.maxsize = 5;
  wflow::workflow wf(ios, opt);

  for (int i = 0; i < 10; ++i)
  {
    wf.post(
      [i](){ std::cout << "post " << i << std::endl; },
      [i](){ std::cout << "drop " << i << std::endl; }
    );
  }

  std::cout << "Run!" << std::endl;
  // Ожидаем выполнение всех заданий
  ios.run();
}
```

Различные варианты таймеров:

```cpp
  boost::asio::io_context ios;
  wflow::workflow wf(ios);

  // простой таймер
  wf.create_timer(
    std::chrono::seconds(5),
    []()->bool{
      std::cout << "Every 5 seconds" << std::endl;
      // Продолжаем работу таймера
      return true;
    }
  );

  // Запускаем пятисекундный таймер через 30 секунд
  wf.create_timer(
    std::chrono::seconds(30),
    std::chrono::seconds(5),
    []()->bool{
      std::cout << "Every 5 seconds" << std::endl;
      return true;
    }
  );

  // Запускаем раз в сутки в 5 утра
  wf.create_timer(
    "05:00:00",
    []()->bool{
      std::cout << "Every day at 5:00 am" << std::endl;
      return true;
    }
  );

  // Первый запуск в 3 утра, далее каждые 6 часов
  wf.create_timer(
    "03:00:00",
    std::chrono::hour(6),
    []()->bool{
      std::cout << "Every six hour" << std::endl;
      return true;
    }
  );

  ios.run();
```

Пример тестирования пропускной способности (порядка 3 млн в секунду) см. example9 и example10
```cpp
  boost::asio::io_context ios;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> wrk(ios);
  wflow::workflow_options opt;
  opt.threads = 1;
  wflow::workflow wf(ios, opt);
  wf.start();
  std::atomic<bool> run(true);
  size_t counter = 0;

  std::thread t([&run, &wf, &counter](){
    for (;run; ++counter)
      wf.post([](){});
  });

  wf.safe_post( std::chrono::seconds(10), [&ios, &run](){ ios.stop(); run = false;});
  ios.run();
  t.join();
  std::cout << counter/10 << std::endl;
```
