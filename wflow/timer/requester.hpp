#pragma once

#include <memory>
#include <functional>
#include <wflow/queue/bique.hpp>

namespace wflow{

class requester
{
public:
  typedef std::function<void(bool)> timer_callback;
  
  template<typename Req, typename Res>
  struct sender_t
  {
    typedef std::function<void(std::unique_ptr<Res>)> callback;
    typedef std::function<bool(std::unique_ptr<Req>, callback)> type;
  };

  template<typename Req, typename Res>
  struct generator_t
  {
    typedef std::function<std::unique_ptr<Req>(std::unique_ptr<Res>)> type;
  };
  
  template<typename Req, typename Res, typename Q>
  static auto make(
    const std::shared_ptr<Q>& q,
    typename sender_t<Req, Res>::type sender,
    typename generator_t<Req, Res>::type generator
  ) -> std::function<void(timer_callback)>
  {
    std::weak_ptr<Q> wq = q;
    return requester::make_requester_<Req, Res>( wq, sender, generator);
  }

private:
  
  template< typename Req, typename Res, typename Q>
  static auto make_requester_(
    std::weak_ptr<Q>& wq,
    typename sender_t<Req, Res>::type sender,
    typename generator_t<Req, Res>::type generator
  ) -> std::function<void(timer_callback)>
  {
    return [wq, sender, generator](timer_callback tc)
    {
      auto req = generator( nullptr );
      requester::send_request_<Req, Res>( wq, std::move(req), sender, generator, tc );
    };
  }

  template< typename Req, typename Res, typename Q>
  static void send_request_(
    std::weak_ptr<Q> wq, 
    std::unique_ptr<Req> req,
    typename sender_t<Req, Res>::type sender,
    typename generator_t<Req, Res>::type generator,
    timer_callback tc
  )
  {
    auto callback = [ wq, sender, generator, tc]( std::unique_ptr<Res> res)
    {
      auto pq = wq.lock();
      if ( pq==nullptr )
        return;

      if ( res == nullptr )
        return tc(true);

      auto pres = std::make_shared< std::unique_ptr<Res> >( std::move(res) );

      pq->post(
          [pres, wq, sender, generator, tc]()
          {
            if ( auto next_req = generator( std::move(*pres) ) )
            {
              send_request_<Req, Res>( wq, std::move(next_req), sender, generator, tc );
            }
            else
            {
              tc(true);
            }
          }, 
          // drop from queue
          [tc]()
          {
            // service unavailable
            tc(true);
          }
      ); // post
    }; // callback
    
    if ( !sender( std::move(req), callback ) )
      tc(false);
  }
};

} // namespace
