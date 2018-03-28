Библиотека на базе boost::asio::io_service для работы с потоками и очередями, с возможностью динамического реконфигурирования и удобными таймерами. 

Гарантированное выполнение заданий:
```cpp
  boost::asio::io_service ios;
  wflow::workflow wf(ios);

  // простая отправка задания (аналогично ios.post(), если не установлен post_delay_ms в опциях )
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );

  // выполнение задания через 4 секунды ( post_delay_ms в опциях игнорируется )
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );

  // выполнение задания в указанное время ( delay_ms в опциях игнорируется )
  auto now = std::chrono::system_clock::now();
  now += std::chrono::seconds(2);
  wf.safe_post( now, [](){ std::cout << "Safe post in time point" << std::endl; } );

  ios.run();
```

По умолчанию, для методов post размер очереди ограничен 512. При превышении размера, часть сообщений будет сбрасываться:
```cpp
  boost::asio::io_service ios;
  wflow::workflow wf(ios);

  // простая отправка задания с контролем размера очереди, без drop-обработчика 
  if ( !wf.post( [](){ std::cout << "Simple unsafe post" << std::endl; }, nullptr ) )
    std::cout << "ERROR: queue overflow!!!" << std::endl;

  // Если не установлено ограние rate_limit, то при переполнении drop-обработчик выполнениться синхронно 
  wf.post( 
    [](){ std::cout << "Simple unsafe post" << std::endl;},
    [](){ std::cout << "ERROR: queue overflow!!!" << std::endl;} 
  );

  // Для отложенных заданий, drop-обработчик выполнениться в момент постановки в очередь, если она переполненна 
  wf.post(
    std::chrono::seconds(4), 
    [](){ std::cout << "Simple unsafe post" << std::endl;},
    [](){ std::cout << "ERROR: queue overflow!!!" << std::endl;} 
  );

  ios.run();
```

Различные варианты таймеров:

```cpp
  boost::asio::io_service ios;
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

По мимо максимального размера очереди, можно также указать размер, при достижении которого, workflow будет отправлять сообщения в лог. 
Чтобы не забивать логи, можно указать интервал проверки или задать собственный обработчик.
