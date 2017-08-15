#pragma once

#include <iow/io/aux/write_buffer_options.hpp>
#include <iow/io/types.hpp>
#include <iow/memory.hpp>
#include <vector>
#include <memory>
#include <string>


namespace iow{ namespace io{


class write_buffer
{
public:
  typedef write_buffer_options options_type;
  typedef std::shared_ptr<options_type> options_ptr;
  typedef typename data_type::value_type value_type;
  typedef const value_type* value_ptr;
  typedef std::unique_ptr<value_type[]> sep_ptr;
  typedef std::pair<value_ptr, size_t> data_pair;
  typedef std::deque<data_ptr> data_list;
  
  typedef std::function<data_ptr(size_t, size_t)> create_fun;
  typedef std::function<void(data_ptr)> free_fun;

  
public:
  
  write_buffer() noexcept;

  void clear();

  
  template<typename O>
  void set_options(const O& opt) noexcept;

  template<typename O>
  void get_options(O& opt) const noexcept;

  size_t size() const noexcept;

  size_t count() const noexcept;
  
  size_t offset() const noexcept;

  size_t capacity() const noexcept;

  bool overflow() const noexcept;
  
  bool ready() const;

  bool waiting() const;

  void attach(data_ptr d);

  data_pair next();

  bool confirm(data_pair p);

  void rollback();
  
  

private:
  
  void addsep_( data_type& d, bool reserve );

  data_ptr create_(size_t size, size_t maxbuf) const;

  data_ptr create_(size_t size) const;

  data_ptr create_() const;

  void free_(data_ptr d) const;

  value_ptr cur_ptr_() const;

  size_t cur_size_() const;

private:
  
  // options
  sep_ptr _sep = nullptr;
  size_t _sep_size = 0;
  size_t _bufsize = 0;
  size_t _maxbuf = 0;
  size_t _minbuf = 0;
  size_t _maxsize = 0;
  bool _first_as_is = false;
  create_fun _create = nullptr;
  free_fun _free = nullptr;

  // -------------------
  size_t _size = 0;
  size_t _offset = 0;
  size_t _wait = 0; // в байтах

  data_list _list;
};

  template<typename O>
  void write_buffer::set_options(const O& opt) noexcept
  {
    if ( opt.sep.empty() )
    {
      _sep = nullptr;
      _sep_size=0;
    }
    else
    {
      _sep_size = opt.sep.size();
      _sep=sep_ptr(new value_type[_sep_size]);
      std::copy(opt.sep.begin(), opt.sep.end(), _sep.get() );
    }

    _bufsize = opt.bufsize;
    _maxbuf = opt.maxbuf;
    _minbuf = opt.minbuf;
    _maxsize= opt.maxsize;
    _first_as_is = opt.first_as_is;

    _create = opt.create;
    _free = opt.free;

    if ( _bufsize == 0 )
    {
      _bufsize = 4096;
    }
    
    if ( _minbuf==0 )
    {
      _minbuf=128;
    }

    if ( _minbuf > _bufsize )
    {
      _minbuf = _bufsize;
    }

    if ( _maxbuf < _bufsize )
    {
      _maxbuf = _bufsize;
    }

    //_list.reserve(2);
  }

  template<typename O>
  void write_buffer::get_options(O& opt) const noexcept
  {
    if ( _sep_size!=0 )
    {
      opt.sep.assign(_sep.get(), _sep.get() + _sep_size);
    }
    else
    {
      opt.sep.clear();
    }
    
    opt.bufsize = _bufsize;
    // opt.maxsize = _maxsize;
    opt.maxbuf  = _maxbuf;
    opt.minbuf  = _minbuf;
    opt.first_as_is = _first_as_is;
    opt.create  = _create;
    opt.free    = _free;
    opt.maxsize=_maxsize;
  }

}}
