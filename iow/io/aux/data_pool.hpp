#pragma once

#include <iow/io/aux/data_pool_options.hpp>
#include <vector>
#include <memory>
#include <iow/mutex.hpp>
#include <iow/memory.hpp>
#include <iostream>

namespace iow{ namespace io{

/*struct data_pool_options
{
  size_t poolsize = 1024;
  size_t minbuf = 128;
  size_t maxbuf = 4096;
};
*/

  
template<typename DataType, typename MutexType = std::mutex >
class data_pool
{
public:
  typedef DataType data_type;
  typedef std::unique_ptr<data_type>  data_ptr;
  typedef std::vector<data_ptr> pool_type;
  typedef MutexType mutex_type;
  typedef data_pool_options options_type;

  /*data_pool(data_pool&&)  =delete;
  data_pool& operator=(data_pool&&)  =delete;
  */
  
  data_pool() noexcept
    : _poolsize(1024)
    , _minbuf(128)
    , _maxbuf(4096)
  {
    //_pool.reserve(_poolsize);
  }
  
  template<typename O>
  void set_options(O&& opt) noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    _poolsize = opt.poolsize;
    _minbuf = opt.minbuf;
    _maxbuf = opt.maxbuf;
    if ( _minbuf > _maxbuf )
      _minbuf = _maxbuf;
    if ( _poolsize == 0 )
        abort();

  }

  template<typename O>
  void get_options(O& opt) const noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    opt.poolsize = _poolsize;
    opt.minbuf = _minbuf;
    opt.maxbuf = _maxbuf;
  }

  /*
  data_ptr create()
  {
    std::lock_guard<mutex_type> lk(_mutex);
    return this->create_(_minbuf, _maxbuf);
  }

  data_ptr create(size_t bufsize)
  {
    std::lock_guard<mutex_type> lk(_mutex);
    return this->create_(bufsize, _maxbuf);
  }
  */
  
  data_ptr create(size_t bufsize, size_t maxbuf ) noexcept
  {
    if ( maxbuf == 0 )
      maxbuf = _maxbuf;
    std::lock_guard<mutex_type> lk(_mutex);
    return this->create_(bufsize, maxbuf);
  }
  
  void free(data_ptr d) noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( d==nullptr || d->empty() )
      return;

    //std::cout << "free " << d->capacity() << std::endl;
    if ( _pool.size() >= _poolsize)
    {
      //std::cout << "free1 " << _pool.size() << " >= " << _poolsize << " " << reinterpret_cast<size_t>(this) << std::endl;
      //std::cout.flush();
      if (_poolsize==0)
        abort();
      return;
    }
    
    if ( d->capacity() < _minbuf || d->capacity() > _maxbuf)
    {
      std::cout << "free2 _maxbuf=" << _maxbuf << std::endl;
      return;
    }
    
    if ( _pool.capacity() < _poolsize )
      _pool.reserve( _poolsize );

  //std::cout << "free3 " << std::endl;    
    _pool.push_back( std::move(d));
  }
  
  size_t count() const noexcept 
  {
    std::lock_guard<mutex_type> lk(_mutex);
    return _pool.size();
  }
  
  size_t capacity() const noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    size_t result = 0;
    for (auto& buf : _pool)
    {
      result += buf.capacity();
    }
    return result;
  }
  
private:
  
  data_ptr create_(size_t bufsize, size_t maxbuf ) noexcept
  {
    if (_pool.empty() )
    {
      //std::cout << "create1 " << maxbuf << std::endl;
      if ( maxbuf==156)
        abort();
      auto p = std::make_unique< data_type >(maxbuf);
      p->resize(bufsize);
      return std::move(p);
    }
    
    auto& buf = _pool.back();
    if ( buf->capacity() < bufsize || buf->capacity() > maxbuf )
    {
      std::cout << "create2 " << bufsize << " buf " << buf->capacity() << std::endl;
      auto p = std::make_unique< data_type >(maxbuf);
      p->resize(bufsize);
      return std::move(p);
    }
//    std::cout << "create " << bufsize <<   " from pool " << buf->capacity() << std::endl;
    buf->resize(bufsize);
    auto result = std::move(buf);
    _pool.pop_back();
    return std::move(result);
  }
  
  pool_type _pool;
  size_t _poolsize;
  size_t _minbuf;
  size_t _maxbuf;
  mutable MutexType _mutex;
};


template<typename DataType, typename MutexType = std::mutex >
class pool_map
{
public:
  typedef MutexType mutex_type;
  typedef pool_map<DataType, MutexType> self;
  typedef std::shared_ptr<self> self_ptr;
  typedef std::function< data_ptr(size_t, size_t) > create_fun;
  typedef std::function< void(data_ptr) > free_fun;

  
  typedef data_pool<DataType, empty_mutex> pool_type;
  typedef std::vector<pool_type> pool_list;
  
  pool_map() { }
  
  template<typename O>
  void get_options(O& opt) const noexcept
  {
    _def.get_options(opt);
  }

  template<typename O>
  void set_options(O opt) noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( opt.maxbuf < opt.minbuf )
      opt.maxbuf = opt.minbuf;
    _minbuf = opt.minbuf;
    _maxbuf = opt.maxbuf;

    _list = pool_list(opt.maxbuf+1);
    /*if ( opt.maxbuf > opt.minbuf )
      _list = pool_list(opt.dimension);
      */

    _def.set_options(opt);
    for (auto& p : _list)
      p.set_options(opt);
  }
  
  /*
  size_t index( size_t bufsize ) const
  {
    size_t diff = _maxbuf - _minbuf;
    if ( diff == 0 )
      return -1;
    size_t count = _list.size() / diff;
    if ( count == 0 )
      return -1;
    size_t size = diff / count;
    if ( diff == 0 )
      return -1;
    return (bufsize - _minbuf)/size;
  }
  */

  data_ptr create(size_t bufsize, size_t maxbuf )
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( maxbuf > _maxbuf )
    {
      std::cout << "DEF" << std::endl;
      return _def.create(bufsize, maxbuf);
    }
    return _list.at( maxbuf/10 ).create(bufsize, maxbuf);
  }
  
  /*
  data_ptr create(size_t bufsize, size_t maxbuf )
  {
    std::lock_guard<mutex_type> lk(_mutex);
    std::cout << "create index: " << this->index(bufsize) << " bufsize=" << bufsize << " minbuf=" << _minbuf << " maxbuf=" <<_maxbuf << std::endl;
    if ( _list.empty() || bufsize < _minbuf || bufsize > _maxbuf )
    {
      std::cout << "def create _list.size()==" << _list.size() << std::endl;
      std::cout.flush();
      return _def.create(bufsize, maxbuf);
    }
    std::cout << "create [" << this->index(bufsize) << "]" << std::endl;
    std::cout.flush();
    return _list[ this->index(bufsize) ].create(bufsize, maxbuf);
  }
  */
  
  /*
  void free(data_ptr d) noexcept
  {
    if ( d==nullptr )
      return;

    std::lock_guard<mutex_type> lk(_mutex);
    
    size_t bufsize = d->capacity();
    std::cout << "free index: " << this->index(bufsize) << std::endl;
    if ( _list.empty() || bufsize < _minbuf || bufsize > _maxbuf )
    {
      
      std::cout << "def free bufsize = " << bufsize << " _list.size()=" << _list.size() << std::endl;
      std::cout.flush();
      _def.free( std::move(d) );
    }
    else
    {
      
      std::cout << "free "<< bufsize << " [" << this->index(bufsize) << "]" << std::endl;
      std::cout.flush();
      _list[ this->index(bufsize) ].free(std::move(d));
    }
  }
  */
  
  void free(data_ptr d) noexcept
  {
    
    if ( d==nullptr || d->empty() )
      return;

    std::lock_guard<mutex_type> lk(_mutex);
    
    size_t bufsize = d->capacity();
  
    if ( bufsize > _maxbuf )
    {
      std::cout << "no free" << std::endl;
      return;
    }
    
    return _list.at( bufsize/10 ).free( std::move(d) );
  }

  /*
  create_fun get_create()
  {
    if ( auto g = self::global() )
    {
      using namespace std::placeholders;
      return std::bind( &self::create, g, _1, _2);
    }
    return nullptr;
  }
  
  free_fun get_free()
  {
    if ( auto g = self::global() )
    {
      using namespace std::placeholders;
      return std::bind(&self::free, g, _1);
    }
  }

  static self_ptr global()
  {
    return s_global_pool;
  }
  
  template<typename Opt>
  static void initialize(Opt opt)
  {
    if ( opt.disabled == false )
    {
      s_global_pool = std::make_shared<self>();
      s_global_pool->set_options(opt);
    }
    else
      s_global_pool = nullptr;
  }
  */
private:
  
  //static self_ptr s_global_pool;
  size_t _minbuf;
  size_t _maxbuf;
  pool_list _list;
  pool_type _def;
  mutable MutexType _mutex;
};


}}
