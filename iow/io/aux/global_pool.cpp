#include "global_pool.hpp"
#include "data_pool.hpp"
#include <iow/mutex/spinlock.hpp>
#include <iow/mutex.hpp>
#include <iow/memory.hpp>

namespace iow{ namespace io{

  /*
typedef pool_map< data_type, spinlock > pool_type;
typedef std::shared_ptr<pool_type> pool_ptr;
static pool_ptr static_pool;
*/
  
void global_pool::initialize(data_map_options /*opt*/)
{
  /*
  if ( static_pool == nullptr )
    static_pool = std::make_shared<pool_type>();
  static_pool->set_options(opt);
  */
}

data_ptr global_pool::create(size_t bufsize, size_t maxbuf)
{
  /*if ( static_pool == nullptr )
  {*/
    auto p = std::make_unique<data_type>(maxbuf);
    p->resize(bufsize);
    return std::move(p);
  /*}
  return static_pool->create(bufsize, maxbuf);*/
}

void global_pool::free(data_ptr )
{
  /*
  if ( static_pool == nullptr )
    return;
  return static_pool->free( std::move(d) );
  */
}

create_fun global_pool::get_create()
{
  return nullptr;
  /*if ( static_pool == nullptr )
    return nullptr;
  using namespace std::placeholders;
  return std::bind( &pool_type::create, static_pool, _1, _2);
  */
}

free_fun global_pool::get_free()
{
  return nullptr;
  /*
  if ( static_pool == nullptr )
    return nullptr;
  using namespace std::placeholders;
  return std::bind( &pool_type::free, static_pool, _1);
  */
}

}}
