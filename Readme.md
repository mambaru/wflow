Библиотека на базе boost::asio::io_service для работы с потоками и очередями, с возможностью динамического реконфигурирования и удобными таймерами. 

Гарантированная отправка сообщений:
```cpp
  boost::asio::io_service ios;
  wflow::workflow wf(ios);

  // простая отправка сообщения (аналогично ios.post(), если не установлен post_delay_ms в опциях )
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );

  // отправка сообщения через 4 секунды ( post_delay_ms в опциях игнорируется )
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );

  // отправка сообщения в указанное время ( delay_ms в опциях игнорируется )
  auto now = std::chrono::system_clock::now();
  now += std::chrono::seconds(2);
  wf.safe_post( now, [](){ std::cout << "Safe post in time point" << std::endl; } );

  ios.run();
```

Если ограничить размер очереди, то при превышении размера, новые сообщения не будут отправлятся:
```cpp
  boost::asio::io_service ios;

  wflow::options opt;
  // максимальный размер очереди
  opt.maxsize = 128;
  // размер очереди, при которой записывается предупреждение в лог
  opt.wrnsize = 64;

  wflow::workflow wf(ios, opt);

  // отправка сообщения с контролем размера очереди
  if ( !wf.post( [](){ std::cout << "Simple unsafe post" << std::endl; }, nullptr ) )
    std::cout << "Drop  handler if queue overflow" << std::endl;
```
По мимо максимального размера очереди, можно также указать размер, при достижении которого, workflow будет отправлять сообщения в лог. 
Чтобы не забивать логи, можно указать интервал проверки или задать собственный обработчик
