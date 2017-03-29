#include <iow/io/aux/read_buffer.hpp>
#include <iow/memory.hpp>
#include <iow/io/types.hpp>
#include <queue>
#include <memory>
#include <algorithm>
#include <iostream>


namespace iow{ namespace io{

  read_buffer::read_buffer()
  {
    _buffers.reserve(2);
  }

  void read_buffer::clear() noexcept
  {
    _size = 0;
    _offset = 0;
    _readbuf = -1;
    _readpos = -1;
    _parsebuf = 0;
    _parsepos = 0;
    _buffers.clear();
  }

  size_t read_buffer::count() const noexcept
  {
    return _buffers.size();
  }

  size_t read_buffer::size() const noexcept
  {
    return _size;
  }

  size_t read_buffer::capacity() const noexcept
  {
    size_t result = 0;
    for (auto& buf: _buffers)
      if ( buf != nullptr)
        result += buf->capacity();
    return result;
  }

  bool read_buffer::waiting() const noexcept
  {
    return _readbuf!=npos();
  }

  bool read_buffer::overflow() const noexcept
  {
    return _maxsize!=0 && this->capacity() > _maxsize;
  }
  
  read_buffer::data_pair read_buffer::next()
  {
    data_pair result(0,0);

    if ( this->waiting() )
      return result;

    if ( _buffers.empty() )
      return create_for_next_();


    data_ptr& last = _buffers.back();

    size_t reserve = last->capacity() - last->size();
    if ( reserve > _minbuf && _minbuf!=0)
    {
      size_t nextsize = _bufsize < reserve ? _bufsize : reserve;
      _readbuf = _buffers.size() - 1;
      _readpos = last->size();
      last->resize( last->size() + nextsize  );
      result.first = &((*last)[_readpos]);
      result.second = nextsize;
    }
    else
    {
      result = create_for_next_();
    }

    return result;
  }

  bool read_buffer::rollback(data_pair d)
  {
    d.second = 0;
    return this->confirm(d);
  }

  bool read_buffer::confirm(data_pair d)
  {
    std::cout << "read_buffer::confirm [" << std::string( d.first, d.first + d.second) << "]" << std::endl;
    if ( !this->waiting() )
      return false;

    if ( _readbuf >= _buffers.size() )
      return false;

    auto& buf = _buffers[_readbuf];

    if ( d.second > 0 )
    {
      if ( &( (*buf)[0]) + _readpos - d.first != 0 )
        return false;

      if ( buf->size() < _readpos + d.second )
        return false;

      _size += d.second;
    }

    buf->resize( _readpos + d.second );
    if ( buf->empty() )
    {
      free_( std::move(buf) );
      _buffers.erase( _buffers.begin() + _readbuf );
    }
    _readpos = -1;
    _readbuf = -1;

    std::cout << "read_buffer::confirm DONE" << std::endl;
    return true;
  }

  data_ptr read_buffer::detach()
  {
    std::cout << "read_buffer::detach() -1.1- " << std::endl;
    std::cout << "_offset= " << _offset << " _parsebuf=" << _parsebuf << " _parsepos=" << _parsepos << std::endl;
    if ( _buffers.empty() )
      return nullptr;
    std::cout << "read_buffer::detach() -1.2- [" << (_sep==nullptr) << "]" << std::endl;
    auto res = search_();
    std::cout << "read_buffer::detach() -1.3- " << std::endl;
    if ( res.first == this->npos() )
    {
      if ( _readbuf != this->npos() )
      {
        std::cout << "read_buffer::detach() -1.4- " << std::endl;
        _parsebuf = _readbuf;
        _parsepos = _readpos;
      }
      else
      {
        std::cout << "read_buffer::detach() -1.5- " << std::endl;
        _parsebuf = _buffers.size()-1;
        _parsepos = _buffers.back()->size();
      }
      std::cout << "read_buffer::detach() -1.6- " << std::endl;
      return nullptr;
    }

    std::cout << "read_buffer::detach() -1.7- " << std::endl;
    auto resbuf = this->make_result_(res);
    this->prepare_(res);
    size_t bufsize = resbuf->size();
    _size -= bufsize;
    if ( _trimsep && bufsize >= _sep_size )
    {
      std::cout << "read_buffer::detach() -1.8- " << std::endl;
      resbuf->resize( bufsize - _sep_size);
    }
    std::cout << "read_buffer::detach() -1.9- " << std::endl;
    std::cout << "_offset= " << _offset << " _parsebuf=" << _parsebuf << " _parsepos=" << _parsepos << std::endl;
    return std::move(resbuf);
  }

  constexpr size_t read_buffer::npos()
  {
    return ~0;
  }

  data_ptr read_buffer::create_(size_t size, size_t maxbuf) const noexcept
  {
    if ( _create!=nullptr )
    {
      return _create(size, maxbuf);
    }

    try
    {
      return std::make_unique<data_type>(size);
    }
    catch(const std::bad_alloc& )
    {
      return nullptr;
    }
  }

  data_ptr read_buffer::create_(size_t size) const noexcept
  {
    return this->create_(size, _bufsize);
  }

  data_ptr read_buffer::create_() const noexcept
  {
    return this->create_(_bufsize);
  }
  
  void read_buffer::free_(data_ptr d) const
  {
    if ( _free != nullptr)
      _free( std::move(d) );
  }

  /**************************************************************************/
  /***************************** next helper ******************************/
  /**************************************************************************/

  read_buffer::data_pair read_buffer::create_for_next_()
  {
    auto ptr = create_();
    
    if ( ptr==nullptr )
      return data_pair(nullptr, -1);

    // Если закончили парсить на последнем буфере
    if ( !_buffers.empty() 
          && _parsebuf == _buffers.size() - 1
          && _buffers[_parsebuf]->size() == _parsepos )
    {
      _parsebuf = _buffers.size();
      _parsepos = 0;
    }

    _buffers.push_back( std::move(ptr) );
    _readbuf = _buffers.size()-1;
    _readpos = 0;
    data_ptr& last = _buffers.back();
   
    return data_pair( &((*last)[0]), last->size());
  }

  /**************************************************************************/
  /***************************** detach helper ******************************/
  /**************************************************************************/

  size_t read_buffer::last_buff_() const
  {
    if ( _readbuf==read_buffer::npos() || (_readpos > 0 && _readpos!=read_buffer::npos()) )
    {
      return _buffers.size() - 1;
    }
    else if (_buffers.size() > 1 )
    {
      return _buffers.size() - 2;
    }
    return -1;
  }

  read_buffer::const_iterator read_buffer::begin_(size_t pos) const
  {
    const_iterator itr = _buffers[pos]->begin();
    if ( pos == 0 )
      itr += _offset;
    return itr;
  }

  read_buffer::const_iterator read_buffer::end_(size_t pos) const
  {
    if ( pos == _readbuf )
    {
      return _buffers[pos]->begin() + _readpos;
    }
    return _buffers[pos]->end();
  }

  read_buffer::const_iterator read_buffer::last_(size_t pos) const
  {
    auto& buf = *(_buffers[pos]);
    if ( pos == _readbuf )
    {
      if (_readpos==0)
      {
        abort();
      }
      return buf.begin() + _readpos - 1;
    }

    if ( buf.empty() )
    {
      abort();
    }
    return buf.begin() + buf.size() - 1;
  }

  void read_buffer::dec_(size_t& pos, const_iterator& itr) const
  {
    auto beg = begin_(pos);
    if ( beg != itr )
    {
      --itr;
    }
    else
    {
      if ( pos == 0 )
      {
        pos = -1;
      }
      else
      {
        --pos;
        itr = last_(pos);
      }
    }
  }

  bool read_buffer::check_sep_( size_t pos, const_iterator itr) const
  {
    if ( _sep_size < 2)
      return true;

    dec_(pos, itr);
    if ( pos == npos() )
      return false;

    if ( _sep_size == 2 )
      return *itr == _sep[0];

    value_type* sbeg = _sep.get();
    value_type* scur = sbeg + _sep_size - 2;
    if ( *itr != *scur )
    {
      return false;
    }
    else if (_sep_size == 2)
    {
      return true;
    }

    for(;;)
    {
      --scur;
      dec_(pos, itr);
      if ( pos == npos() )
        return false;
      if ( *itr != *scur )
        return false;
      if (scur==sbeg)
        return true;
    }
  }

  read_buffer::search_pair read_buffer::nosep_search_() const
  {
    std::cout << "nosep_search_ -1-" << std::endl;
    if ( _offset!=0 && _parsebuf==0 && _offset==_parsepos ) 
    {
      std::cout << "nosep_search_ -2-" << std::endl;
      return search_pair(-1, -1);
    }

    std::cout << "nosep_search_ -3-" << std::endl;
    if ( _readbuf==npos() )
    {
      std::cout << "nosep_search_ -4-" << std::endl;
      // Если последний буфер не выделен под чтение
      return search_pair(_buffers.size() - 1, _buffers.back()->size());
    }
    else if (_readpos != 0)
    {
      std::cout << "nosep_search_ -5-" << std::endl;
      // Если последний буфер выделен под чтение, но не сначала
      return search_pair(_readbuf, _readpos);
    }
    else if ( _readbuf != 0 )
    {
      std::cout << "nosep_search_ -6-" << std::endl;
      // Если последний буфер выделен под чтение с начала
      // и он не первый
      return search_pair( _readbuf-1, _buffers[_readbuf-1]->size());
    }
    std::cout << "nosep_search_ -7-" << std::endl;
    // Нет готовых данных
    return search_pair(-1, -1);
  }

  read_buffer::search_pair read_buffer::search_() const
  {
    if ( _buffers.empty() || _parsebuf==npos() )
      return search_pair(-1, -1);

    if (_sep_size==0)
      return nosep_search_();

    // Если ожидаем confirm() для парсинга
    if (_readbuf==_parsebuf && _readpos==_parsepos)
      return search_pair(-1, -1);

    // Если нужен next() confirm() для парсинга
    if (_buffers[_parsebuf]->size() == _parsepos )
      return search_pair(-1, -1);

    // Если последний буфер выделен полностью для чтения, то игнорируем его
    size_t toparse = _buffers.size() - (_readbuf!=npos() && _readpos==0);

    for (size_t i=_parsebuf; i < toparse; ++i)
    {
      const_iterator end = end_(i);
      const_iterator beg;
      if ( i==_parsebuf )
      {
        beg = _buffers[i]->begin() + _parsepos;
      }
      else
      {
        beg = begin_(i);
      }

      while ( beg!=end )
      {
        beg = std::find(beg, end, _sep[_sep_size-1]);

        if ( beg == end )
          break;

        if ( check_sep_(i, beg) )
        {
          return search_pair( i, std::distance(_buffers[i]->cbegin(), beg) + 1 );
        }
        ++beg;
      }
    }
    return search_pair(-1, -1);
  }

  // Удаляем отработанные буферы и настраиваем состояние
  void read_buffer::prepare_(const read_buffer::search_pair& p) 
  {
    if ( p.first==0 )
    {
      if ( _buffers[0] == nullptr )
      {
        _parsepos = 0;
        _parsebuf = 0;
        free_(std::move( _buffers.front() ) );
        _buffers.erase( _buffers.begin() );
        _offset = 0;

        if (_readbuf != npos())
        {
          if (_readbuf==0)
            abort();
          --_readbuf;
        }
      }
      else
      {
        _offset = p.second;
        _parsepos = p.second;
        _parsebuf = 0;
      }
    }
    else
    {
      bool complete = _buffers[p.first]->size() == p.second;
      size_t off = p.first + complete;
      if ( off > 0 )
      {
        std::for_each(_buffers.begin(), _buffers.begin() + off, [this](data_ptr& d){ this->free_( std::move(d) );});
        _buffers.erase( _buffers.begin(), _buffers.begin() + off );
        if (_readbuf != npos())
        {
          if (_readbuf < off)
            abort();
          _readbuf-=off;
        }
      }
      _parsebuf = 0;
      _offset = complete ? 0 : p.second;
      _parsepos = _offset;
    }
    
    if ( _buffers.size() > 128 &&  _buffers.size()*2 < _buffers.capacity() )
      _buffers.shrink_to_fit();

  }

  data_ptr read_buffer::make_result_(const search_pair& p)
  {
    data_ptr result = make_result_if_first_(p);

    // Если блок готов в первом буфере
    if ( result != nullptr )
      return std::move(result);

    // Расчитываем необходимый резерв
    size_t reserve = 0;
    for (size_t i=0; i < p.first + 1; ++i)
    {
      reserve+=_buffers[i]->size();
    }

    // reserve с небольшим оверхедом, поэтому очищаем и используем inserter
    result = create_(reserve, reserve*2 < _maxbuf ? reserve*2 : reserve);
    result->clear();

    // Копируем со всех буферов, что готовы
    for (size_t i=0; i < p.first + 1; ++i)
    {
      if ( i != p.first )
      {
        std::copy(begin_(i), end_(i), std::inserter(*result, result->end()));
      }
      else
      {
        std::copy(begin_(i), _buffers[i]->cbegin() + p.second, std::inserter(*result, result->end()));
      }
    }

    return std::move(result);
  }

  data_ptr read_buffer::make_result_if_first_(const search_pair& p)
  {
    if ( p.first!=0 )
      return nullptr;

    data_ptr result = nullptr;

    // Если можем полностью захватить буфер
     size_t tmp = _buffers[0]->size();
    if ( tmp == p.second )
    {
      result = std::move(_buffers[0]);
      // Если ральные данные не с начала буфера
      if (_offset!=0)
      {
        result->erase(result->begin(), result->begin() + _offset);
      }
    }
    else
    {
      size_t size = p.second - _offset;
      result = create_(p.second - _offset, size*2 < _maxbuf ? size*2 : size);
      std::copy(_buffers[0]->begin() + _offset, _buffers[0]->begin() + p.second, result->begin() );
    }
    return std::move(result);
  }

}}
