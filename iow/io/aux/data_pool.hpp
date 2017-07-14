#pragma once

#include <iow/io/aux/data_pool_options.hpp>
#include <vector>
#include <memory>
#include <iow/mutex.hpp>
#include <iow/memory.hpp>
#include <iostream>

namespace iow{ namespace io{

  
template<typename DataType, typename MutexType = std::mutex >
class data_pool
{
public:
  typedef DataType data_type;
  typedef std::unique_ptr<data_type>  data_ptr;
  typedef std::vector<data_ptr> pool_type;
  typedef MutexType mutex_type;
  typedef data_pool_options options_type;
  
  
  template<typename O>
  void set_options(O&& opt) noexcept
  {
    //std::lock_guard<mutex_type> lk(_mutex);
    _poolsize = opt.poolsize;
    _minbuf = opt.minbuf;
    _maxbuf = opt.maxbuf;
    if ( _minbuf > _maxbuf )
      _minbuf = _maxbuf;
  }

  template<typename O>
  void get_options(O& opt) const noexcept
  {
    //std::lock_guard<mutex_type> lk(_mutex);
    opt.poolsize = _poolsize;
    opt.minbuf = _minbuf;
    opt.maxbuf = _maxbuf;
  }

  data_ptr create(size_t bufsize, size_t maxbuf ) noexcept
  {
    if ( maxbuf == 0 )
      maxbuf = _maxbuf;
    //std::lock_guard<mutex_type> lk(_mutex);
    return this->create_(bufsize, maxbuf);
  }
  
  void free(data_ptr d) noexcept
  {
    //std::lock_guard<mutex_type> lk(_mutex);
    if ( d==nullptr )
      return;
    
    if ( _pool.size() >= _poolsize)
    {
      return;
    }
    
    if ( d->capacity() < _minbuf || d->capacity() > _maxbuf)
    {
      return;
    }
    
    if ( _pool.capacity() < _poolsize )
      _pool.reserve( _poolsize );

    _pool.push_back( std::move(d));
  }
  
  size_t count() const noexcept
  {
    //std::lock_guard<mutex_type> lk(_mutex);
    return _pool.size();
  }
  
  size_t capacity() const noexcept
  {
    //std::lock_guard<mutex_type> lk(_mutex);
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
      auto d = std::make_unique< data_type >(maxbuf);
      d->reserve(maxbuf);
      return std::move(d);
    }
    
    auto& buf = _pool.back();
    if ( buf->capacity() < bufsize || buf->capacity() > maxbuf )
    {
      auto d = std::make_unique< data_type >(maxbuf);
      d->reserve(maxbuf);
      return std::move(d);
    }
    
    buf->resize(bufsize);
    auto result = std::move(buf);
    _pool.pop_back();
    return std::move(result);
  }
  
  pool_type _pool;
  size_t _poolsize = 1024;
  size_t _minbuf = 128;
  size_t _maxbuf = 4096;
  //mutable MutexType _mutex;
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
    opt.dimension = _dimension;
    opt.disabled = _disabled;
  }

  template<typename O>
  void set_options(O opt) noexcept
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( opt.maxbuf < opt.minbuf )
      opt.maxbuf = opt.minbuf;
    _dimension = opt.dimension;
    _disabled = opt.disabled;

    _minbuf = opt.minbuf;
    _maxbuf = opt.maxbuf;

    if ( _disabled )
        _list.clear();
    else
      _list.resize(_dimension + 1);
    
    _list.shrink_to_fit();
    _def.set_options(opt);
    for (auto& p : _list)
      p.set_options(opt);
    return;
  }

  size_t index( size_t size ) const
  {
    size_t i = size / (_maxbuf / _dimension);
    return i;
  }
  data_ptr create(size_t bufsize, size_t maxbuf )
  {
    if ( _disabled || _dimension == 0)
    {
      auto p = std::make_unique<data_type>(maxbuf);
      p->resize(bufsize);
      return std::move(p);
    }
    
    std::lock_guard<mutex_type> lk(_mutex);
    if ( maxbuf > _maxbuf )
      return _def.create(bufsize, maxbuf);
    return _list[ this->index(maxbuf) ].create(bufsize, maxbuf);
  }
  
  void free(data_ptr d) noexcept
  {
    if ( d==nullptr || _disabled || _dimension == 0 )
      return;

    std::lock_guard<mutex_type> lk(_mutex);
    
    size_t bufsize = d->capacity();
  
    if ( bufsize > _maxbuf )
      return;
    
    return _list[ this->index(bufsize) ].free( std::move(d) );
  }

private:
  bool _disabled = false;
  size_t _dimension = 1024;
  size_t _minbuf = 0;
  size_t _maxbuf = 4096;
  pool_list _list;
  pool_type _def;
  mutable MutexType _mutex;
};


}}
