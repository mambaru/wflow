#pragma once

#include <wflow/expires_at.hpp>
#include <memory>
#include <chrono>


namespace wflow{

class common_timer
{
public:
  typedef std::weak_ptr<bool> wflag_type;

  typedef std::function<bool()> handler;
  typedef std::function<void(bool)> handler_callback;
  typedef std::function<void(handler_callback)> async_handler;

  typedef std::chrono::system_clock         clock_t;
  typedef std::chrono::time_point<clock_t>  time_point_t;
  typedef std::chrono::time_point< std::chrono::steady_clock >::duration duration_t;


  template<typename Q, typename Handler>
  static std::function<void()> make( const std::shared_ptr<Q>& pq, duration_t delay, const Handler& h, expires_at expires, wflag_type wflag )
  {
    std::weak_ptr<Q> wq = pq;
    return create_handler_(wq, delay, expires, std::move(h), wflag);
  }

private:

  template<typename Q, typename Handler>
  static std::function<void()> create_handler_(std::weak_ptr<Q> wq, duration_t delay, expires_at expires, const Handler& h, wflag_type wflag)
  {
    return [wq, delay, expires, h, wflag]()
    {
      if ( expires == expires_at::after )
      {
        common_timer::expires_after_(wq, delay, h, wflag );
      }
      else
      {
        common_timer::expires_before_(wq, delay, h, wflag );
      }
    };
  }

  template<typename Q>
  static void expires_after_(std::weak_ptr<Q> wq, duration_t delay, const handler& h, wflag_type wflag)
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    if ( *pflag==false || h() )
    {
      if ( auto pq = wq.lock() )
      {
        pq->safe_delayed_post(delay, [wq, delay, h, wflag]()
          {
            common_timer::expires_after_(wq, delay, h, wflag );
          }
        );
      }
    }
  }

  template<typename Q>
  static void expires_after_(std::weak_ptr<Q> wq, duration_t delay, const async_handler& h, wflag_type wflag)
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto post_handler = [wq, delay, h, wflag]()
    {
      common_timer::expires_after_(wq, delay, std::move(h), wflag );
    };
    
    if ( *pflag )
    {
      h( [wq, delay, h, wflag, post_handler](bool ready)
      {
        if ( !ready )
          return;
        
        if ( auto pq = wq.lock() )
        {
          pq->safe_delayed_post(delay, post_handler);
        }
      });
    }
    else
    {
      if ( auto pq = wq.lock() )
      {
        pq->safe_delayed_post(delay, post_handler);
      }
    }
  }

  template<typename Q>
  static void expires_before_(std::weak_ptr<Q> wq, duration_t delay, const handler& h, wflag_type wflag)
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto pq = wq.lock();
    if ( pq == nullptr )
      return;

    // Сначала delayed_post, потом вызов обработчика
    // pres - для отмены, если обработчика вернул false
    std::shared_ptr<bool> pres = std::make_shared<bool>(true);
    std::weak_ptr<bool> wres = pres;
    
    pq->safe_delayed_post(delay, [wq, delay, h, wflag, wres]()
      {
        if ( wres.lock() == nullptr )
          return;
        common_timer::expires_before_(wq, delay, h, wflag );
      }
    );
    
    if ( *pflag==false || !h() )
    {
      pres.reset();
    }
  }

  template<typename Q>
  static void expires_before_(std::weak_ptr<Q> wq, duration_t delay, async_handler h, wflag_type wflag)
  {
    auto pflag = wflag.lock();
    if ( pflag == nullptr )
      return;

    auto pq = wq.lock();
    if ( pq == nullptr )
      return;

    std::shared_ptr<bool> pres = std::make_shared<bool>(true);
    std::weak_ptr<bool> wres = pres;

    pq->safe_delayed_post(delay, [wq, delay, h, wflag, wres]()
      {
        if ( wres.lock() == nullptr )
          return;
        
        common_timer::expires_before_(wq, delay, std::move(h), wflag );
      }
    );
    
    if ( *pflag==true )
    {
      h( [wres](bool ready) mutable
      {
        if (!ready)
          wres.reset();
      });
    }
  }
};

}
