Библиотека на базе boost::asio::io_service для работы с потоками и очередями, с возможностью динамического реконфигурирования и удобными таймерами. 

Безапасная отправка сообщений   
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

Если ограничить размер очереди, то при превышении размера, новые сообщения не будут туда ставиться и будет 
вызван альтернативный обработчик ()
```cpp
  boost::asio::io_service ios;

  wflow::options opt;
  // максимальный размер очереди
  opt.maxsize = 128;
  // размер очереди, при которой записывается предупреждение в лог
  opt.wrnsize = 64;

  wflow::workflow wf(ios, opt);

  // отправка сообщения с контролем размера очереди
  // второй обработчик вызывается, если сообщение выкидывается из очереди 
  wf.post( 
    [](){ std::cout << "Simple unsafe post" << std::endl; },  
    [](){ std::cout << "Drop  handler if queue overflow" << std::endl; }
  );

  // В текущей реализации это эквивалентно 
  if ( !wf.post( [](){ std::cout << "Simple unsafe post" << std::endl; }, nullptr ) )
    std::cout << "Drop  handler if queue overflow" << std::endl;

```
