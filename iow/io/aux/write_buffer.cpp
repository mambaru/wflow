#include <iow/io/aux/write_buffer.hpp>
#include <iow/io/types.hpp>
#include <iow/memory.hpp>
#include <vector>
#include <memory>
#include <string>


namespace iow{ namespace io{


  write_buffer::write_buffer() noexcept
    : _sep(nullptr)
    , _sep_size(0)
    , _bufsize(8*1024)
    , _maxbuf(8*1024)
    , _minbuf(0) 
    , _first_as_is(true)
    , _create(nullptr)
    , _free(nullptr)
    , _size(0)
    , _offset(0)
    , _wait(0)
  {
    _list.reserve(2);
  }

  void write_buffer::clear()
  {
    _size = 0;
    _offset = 0;
    _list.clear();
    _wait = 0;
  }

  
  size_t write_buffer::size() const noexcept
  {
    return _size;
  }

  size_t write_buffer::count() const noexcept
  {
    return _list.size();
  }
  
  size_t write_buffer::offset() const noexcept
  {
    return _offset;
  }

  size_t write_buffer::capacity() const noexcept
  {
    size_t result = 0;
    for ( auto& d : _list )
      result += _list.capacity();

    return result;
  }

  bool write_buffer::ready() const
  {
    return _wait == 0 && !_list.empty();
  }

  bool write_buffer::waiting() const
  {
    return _wait != 0;
  }
  
  void write_buffer::addsep_( data_type& d, bool reserve )
  {
    if (_sep_size==0)
      return;
    
    if (reserve)
      d.reserve( d.size() + _sep_size );
    
    std::copy( _sep.get(), _sep.get() + _sep_size, std::inserter(d, d.end()) );
    _size += _sep_size;
  }

  void write_buffer::attach(data_ptr d)
  {
    if ( d==nullptr || ( d->empty() && _sep_size==0 ) )
      return;

    _size += d->size();
    if ( _list.empty() )
    {
      addsep_(*d, true);
      _list.push_back( std::move(d) );
    }
    else
    {
      data_ptr& last = _list.back();
      size_t sumsize = last->size() + d->size() + _sep_size;
      if ( last->size() < _minbuf && sumsize < _maxbuf )
      {
        last->reserve(sumsize);
        std::copy( d->begin(), d->end(), std::inserter(*last, last->end() ) );
        addsep_(*last, false);
        free_(std::move(d));
      }
      else
      {
        addsep_(*d, true);
        _list.push_back( std::move(d) );
      }
    }
  }

  write_buffer::data_pair write_buffer::next()
  {
    if ( !this->ready() )
      return data_pair();

    auto size = this->cur_size_();
    auto ptr  = this->cur_ptr_();
    _wait = size;

    return data_pair( ptr, size );
  }

  bool write_buffer::confirm(data_pair p)
  {
    bool result = false;

    if ( _wait == 0 || _size < p.second ) 
      return result;

    _wait = 0;
    _offset += p.second;
    _size -= p.second;
    
    if ( _offset == _list.front()->size())
    {
      _offset = 0;
      free_( std::move(_list.front()) );
      //_list.pop_front();
      _list.erase( _list.begin() );
    }
    
    // TODO: убрал копирование для minbuf
    return true;
  }

  void write_buffer::rollback()
  {
    if ( _wait != 0 )
      return;
    _wait = 0;
  }
  

  data_ptr write_buffer::create_(size_t size, size_t maxbuf) const
  {
    if ( _create!=nullptr )
      return _create(size, maxbuf);
    return std::make_unique<data_type>(size);
  }

  data_ptr write_buffer::create_(size_t size) const
  {
    return this->create_(size, _bufsize);
  }

  data_ptr write_buffer::create_() const
  {
    return this->create_(_bufsize);
  }

  void write_buffer::free_(data_ptr d) const
  {
    if ( _free != nullptr)
      _free( std::move(d) );
  }

  write_buffer::value_ptr write_buffer::cur_ptr_() const
  {
    return &(_list.front()->operator[](0)) + _offset;
  }

  size_t write_buffer::cur_size_() const
  {
    size_t size = _list.front()->size() - _offset;
    bool first_as_is = _first_as_is && _offset==0;
    if ( !first_as_is && size > _maxbuf )
    {
      size = _maxbuf;
    }
    return size;
  }

}}
