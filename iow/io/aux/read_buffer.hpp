#pragma once

#include <iow/io/aux/read_buffer_options.hpp>
#include <iow/memory.hpp>
#include <iow/io/types.hpp>
#include <fas/typemanip/empty_type.hpp>
#include <queue>
#include <memory>
#include <deque>
#include <algorithm>


namespace iow{ namespace io{

class read_buffer
{
public:
  typedef typename data_type::value_type value_type;
  typedef read_buffer_options options_type;
  typedef value_type* value_ptr;
  typedef std::unique_ptr<value_type[]> sep_ptr;
  typedef std::ptrdiff_t diff_type;
  typedef std::pair<value_ptr, size_t> data_pair;
  typedef std::function<data_ptr(size_t, size_t)> create_fun;
  typedef std::function<void(data_ptr)> free_fun;
  
private:
  typedef std::vector<data_ptr> buffer_list;
  typedef typename data_type::const_iterator const_iterator;
  typedef std::pair<size_t, size_t> search_pair;

public:

  read_buffer();

  read_buffer(const read_buffer& other) = delete;
  read_buffer(read_buffer&&) = delete;
  read_buffer& operator=(const read_buffer& other) = delete;
  read_buffer& operator=(read_buffer&& other) = delete;

  template<typename O>
  void set_options(const O& opt) noexcept;

  template<typename O>
  void get_options(O& opt) const noexcept;

  void clear() noexcept;

  size_t count() const noexcept;

  size_t size() const noexcept;

  size_t capacity() const noexcept;

  bool waiting() const noexcept;

  bool overflow() const noexcept;

  data_pair next();

  bool rollback(data_pair d);

  bool confirm(data_pair d);

  data_ptr detach();

  
private:

  //static constexpr diff_type npos();

  data_ptr create_(size_t size, size_t maxbuf) const noexcept;

  data_ptr create_(size_t size) const noexcept;

  data_ptr create_() const noexcept;

  void free_(data_ptr d) const;

private:

  /**************************************************************************/
  /***************************** next helper ******************************/
  /**************************************************************************/

  data_pair create_for_next_();

  /**************************************************************************/
  /***************************** detach helper ******************************/
  /**************************************************************************/

//  size_t last_buff_() const;

  const_iterator begin_(size_t pos) const;

  const_iterator end_(size_t pos) const;

  const_iterator last_(size_t pos) const;

  void dec_(size_t& pos, const_iterator& itr) const;

  bool check_sep_( size_t pos, const_iterator itr) const;

  search_pair nosep_search_() const;

  search_pair search_() const;

  // Удаляем отработанные буферы и настраиваем состояние
  void prepare_(const search_pair& p);

  data_ptr make_result_(const search_pair& p);

  data_ptr make_result_if_first_(const search_pair& p);

private:

// options
  sep_ptr _sep = nullptr;
  size_t _sep_size = 0;
  size_t _bufsize = 0;
  size_t _maxbuf = 0;
  size_t _minbuf = 0;
  size_t _maxsize = 0;
  bool   _trimsep = 0;

  create_fun _create = nullptr;
  free_fun _free = nullptr;

  size_t  _size = 0;
  size_t _offset = 0;  // Смещение в первом буфере
  size_t _readbuf = ~0ul; // -1 - если не ожидает подтверждения
  size_t _readpos = ~0ul;
  
  // Номер буфера, с которого продолжить парсинг
  //   при _buffers.empty() равен 0, но в это случае поиск не производится
  size_t _parsebuf = 0;
  // Позиция в буфере, с которого продолжить парсинг
  // Может быть равен _buffers[_parsebuf]->size() для _buffers.size()-1==_parsebuf
  //   в этом случае корректируется при следующем next(), если выделяеться новый буфер
  //   и поиск до confirm() недоступен
  size_t _parsepos = 0;

  buffer_list   _buffers;
};


  template<typename O>
  void read_buffer::set_options(const O& opt) noexcept
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
    _trimsep = opt.trimsep;
    _create = opt.create;
    _free = opt.free;
    _maxsize = opt.maxsize;

    if ( _bufsize == 0 )
    {
      _bufsize = 4096;
    }
    
    if ( _minbuf==0 )
    {
      _minbuf = _maxbuf;
    }

    if ( _minbuf > _bufsize )
    {
      _minbuf = _bufsize;
    }

    if ( _maxbuf < _bufsize )
    {
      _maxbuf = _bufsize;
    }
    
  }

  template<typename O>
  void read_buffer::get_options(O& opt) const noexcept
  {
    if ( _sep_size!=0 )
    {
      opt.sep.assign(_sep.get(), _sep.get() + _sep_size);
    }
    else
    {
      opt.sep.clear();
    }

    opt.maxsize = _maxsize;
    opt.bufsize = _bufsize;
    opt.maxbuf  = _maxbuf;
    opt.minbuf  = _minbuf;
    opt.trimsep = _trimsep;
    opt.create  = _create;
    opt.free    = _free;
  }

}}
