#pragma once

#include <wflow/expires_at.hpp>
#include <memory>
#include <chrono>
#include <atomic>
#include "time_parser.hpp"

namespace wflow{

class basic_timer_handler
{
  typedef basic_timer_handler self;
public:
  typedef std::weak_ptr<bool> wflag_type;

  typedef std::function<bool()> handler;
  typedef std::function<void(bool)> handler_callback;
  typedef std::function<void(handler_callback)> async_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;
  typedef time_parser::cron_t cron_t;

  /**
   * @tparam Dura duration_t or cron_t
   */
  template<typename Q, typename Handler, typename Dura>
  static std::function<void()> make( 
    const std::shared_ptr<Q>& pq, 
    Dura delay, 
    const Handler& h, 
    expires_at expires, 
    wflag_type wflag )
  {
    std::weak_ptr<Q> wq = pq;
    return create_handler_(wq, std::move(delay), expires, std::move(h), wflag);
  }

private:
  
  static duration_t make_delay_(const duration_t& d)
  {
    return d;
  }

  static duration_t make_delay_(const cron_t& c)
  {
    return std::chrono::seconds( time_parser::cron_next(c) - std::time(nullptr) );
  }

  template<typename Q, typename Handler, typename Dura>
  static std::function<void()> create_handler_(
    std::weak_ptr<Q> wq,
    Dura delay,
    expires_at expires,
    const Handler& h, 
    wflag_type wflag
  )
  {
    return [wq, delay, expires, h, wflag]()
    {
      if ( expires == expires_at::after )
      {
        self::expires_after_(wq, std::move(delay), h, wflag );
      }
      else
      {
        self::expires_before_(wq, std::move(delay), h, wflag );
      }
    };
  }

  template<typename Q, typename Dura>
  static void expires_after_(
    std::weak_ptr<Q> wq, 
    Dura delay, 
    const handler& h, 
    wflag_type 
    wflag
  )
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    if ( *pflag==false || h() )
    {
      if ( auto pq = wq.lock() )
      {
        pq->safe_delayed_post(make_delay_(delay), [wq, delay, h, wflag]()
          {
            self::expires_after_(wq, std::move(delay), h, wflag );
          }
        );
      }
    }
  }

  template<typename Q, typename Dura>
  static void expires_after_(
    std::weak_ptr<Q> wq, 
    Dura delay, 
    const async_handler& h, 
    wflag_type wflag
  )
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto post_handler = [wq, delay, h, wflag]()
    {
      self::expires_after_(wq, std::move(delay), std::move(h), wflag );
    };
    
    if ( *pflag )
    {
      h( [wq, delay, h, wflag, post_handler](bool ready)
      {
        if ( !ready )
          return;
        
        if ( auto pq = wq.lock() )
        {
          pq->safe_delayed_post(make_delay_(delay), post_handler);
        }
      });
    }
    else
    {
      if ( auto pq = wq.lock() )
      {
        pq->safe_delayed_post(make_delay_(delay), post_handler);
      }
    }
  }

  template<typename Q, typename Dura>
  static void expires_before_(
    std::weak_ptr<Q> wq, 
    Dura delay, 
    const handler& h, 
    wflag_type wflag
  )
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto pq = wq.lock();
    if ( pq == nullptr )
      return;

    // Сначала delayed_post, потом вызов обработчика
    // pres - для отмены, если обработчика вернул false
    std::shared_ptr<std::atomic_flag> pres = std::make_shared<std::atomic_flag>();
    pres->test_and_set();
    pq->safe_delayed_post(make_delay_(delay), [wq, delay, h, wflag, pres]()
      {
        if ( !pres->test_and_set() )
          return;
        self::expires_before_(wq, delay, h, wflag );
      }
    );
    
    if ( *pflag==false || !h() )
    {
      pres->clear();
    }
  }

  template<typename Q, typename Dura>
  static void expires_before_(
    std::weak_ptr<Q> wq, 
    Dura delay, 
    async_handler h, 
    wflag_type wflag
  )
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto pq = wq.lock();
    if ( pq == nullptr )
      return;

    std::shared_ptr<std::atomic_flag> pres = std::make_shared<std::atomic_flag>();
    pres->test_and_set();

    pq->safe_delayed_post(make_delay_(delay), [wq, delay, h, wflag, pres]()
      {
        if ( !pres->test_and_set() )
          return;
        
        self::expires_before_(wq, delay, std::move(h), wflag );
      }
    );
    
    if ( *pflag==true )
    {
      h( [pres](bool ready) mutable
      {
        if (!ready) pres->clear();
      });
    }
  }
};

}
