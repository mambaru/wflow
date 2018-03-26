Библиотека на базе boost::asio::io_service для работы с потоками и очередями, с возможностью динамического реконфигурирования и удобными таймерами. 

Безапасная отправка сообщений   
```cpp
  boost::asio::io_service ios;
  wflow::workflow wf(ios);

  // простая отправка сообщения (аналогично ios.post(), если не установлен delay_ms в опциях )
  wf.safe_post( [](){ std::cout << "Simple safe post  " << std::endl; } );

  // отправка сообщения через 4 секунды ( delay_ms в опциях игнорируется )
  wf.safe_post( std::chrono::seconds(4), [](){ std::cout << "Safe post after delay 4 second " << std::endl; } );

  // отправка сообщения в указанное время ( delay_ms в опциях игнорируется )
  auto now = std::chrono::system_clock::now();
  now += std::chrono::seconds(2);
  wf.safe_post( now, [](){ std::cout << "Safe post in time point" << std::endl; } );

  // отправка сообщения с контролем размера очереди
  // второй обработчик вызывается, когда сообщение выкидывается из очереди 
  wf.post( 
    [](){ std::cout << "Simple unsafe post" << std::endl; },  
    [](){ std::cout << "Drop  handler if queue overflow" << std::endl; }
  );

  ios.run();
```
