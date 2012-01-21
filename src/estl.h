#ifndef ESTL_H
#define ESTL_H

#include <stdlib.h>
#include <string.h>

template<typename T, typename U> static inline T min (T  a, U b) { return a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline T max (T  a, U b) { return a > (T)b ? a : (T)b; }

template<typename T, typename U> static inline void swap (T& a, U& b) { T t = a; a = (T)b; b = (U)t; }

template <typename I, typename T>
I find (I first, I last, const T& value)
{
  while (first != last && *first != value)
    ++first;

  return first;
}

/* simplevec taken (and heavily modified), from:
 *
 *  MICO --- a free CORBA implementation
 *  Copyright (C) 1997-98 Kay Roemer & Arno Puder
 *  originally GPLv2 or any later
 */
template<class T>
struct simplevec
{
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef unsigned long size_type;

private:
  size_type _last, _size;
  T *_buf;

public:
  const_iterator begin () const { return &_buf[0]; }
  iterator       begin ()       { return &_buf[0]; }

  const_iterator end () const { return &_buf[_last]; }
  iterator       end ()       { return &_buf[_last]; }

  size_type capacity () const { return _size; }
  size_type size     () const { return _last; }

private:
  static T *alloc (size_type n)
  {
    return (T *)::operator new ((size_t) (n * sizeof (T)));
  }

  static void dealloc (T *buf)
  {
    if (buf)
      ::operator delete (buf);
  }

  size_type good_size (size_type n)
  {
    return max (n, _size ? _size * 2 : 5);
  }

  void reserve (iterator where, size_type n)
  {
    if (_last + n <= _size)
      memmove (where + n, where, (end () - where) * sizeof (T));
    else
      {
        size_type sz = _last + n;
        sz = good_size (sz);
        T *nbuf = alloc (sz);

        if (_buf)
          {
            memcpy (nbuf, begin (), (where - begin ()) * sizeof (T));
            memcpy (nbuf + (where - begin ()) + n, where, (end () - where) * sizeof (T));
            dealloc (_buf);
          }

        _buf = nbuf;
        _size = sz;
      }
  }

public:
  void reserve (size_type sz)
  {
    if (_size < sz)
      {
        sz = good_size (sz);
        T *nbuf = alloc (sz);

        if (_buf)
          {
            memcpy (nbuf, begin (), size () * sizeof (T));
            dealloc (_buf);
          }

        _buf = nbuf;
        _size = sz;
      }
  }

  simplevec ()
  : _last(0), _size(0), _buf(0)
  {
  }

  simplevec (size_type n, const T& t = T ())
  : _last(0), _size(0), _buf(0)
  {
    insert (begin (), n, t);
  }

  simplevec (const_iterator first, const_iterator last)
  : _last(0), _size(0), _buf(0)
  {
    insert (begin (), first, last);
  }

  simplevec (const simplevec<T> &v)
  : _last(0), _size(0), _buf(0)
  {
    reserve (v._last);
    memcpy (_buf, v.begin (), v.size () * sizeof (T));
    _last = v._last;
  }

  simplevec<T> &operator= (const simplevec<T> &v)
  {
    if (this != &v)
      {
        _last = 0;
        reserve (v._last);
        memcpy (_buf, v.begin (), v.size () * sizeof (T));
        _last = v._last;
      }

    return *this;
  }

  ~simplevec ()
  {
    dealloc (_buf);
  }

  const T &front () const { return _buf[      0]; }
        T &front ()       { return _buf[      0]; }
  const T &back  () const { return _buf[_last-1]; }
        T &back  ()       { return _buf[_last-1]; }

  bool empty () const
  {
    return _last == 0;
  }

  void clear ()
  {
    _last = 0;
  }

  void push_back (const T &t)
  {
    reserve (_last+1);
    *end () = t;
    ++_last;
  }

  void push_back (T &t)
  {
    reserve (_last+1);
    *end () = t;
    ++_last;
  }

  void pop_back ()
  {
    --_last;
  }

  const T &operator [](size_type idx) const { return _buf[idx]; }
        T &operator [](size_type idx)       { return _buf[idx]; }

  iterator insert (iterator pos, const T &t)
  {
    long at = pos - begin ();
    reserve (pos, 1);
    pos = begin () + at;
    *pos = t;
    ++_last;
    return pos;
  }

  iterator insert (iterator pos, const_iterator first, const_iterator last)
  {
    long n = last - first;
    long at = pos - begin ();

    if (n > 0)
      {
        reserve (pos, n);
        pos = begin ()+at;
        memcpy (pos, first, (last - first) * sizeof (T));
        _last += n;
      }

    return pos;
  }

  iterator insert (iterator pos, size_type n, const T &t)
  {
    long at = pos - begin ();

    if (n > 0)
      {
        reserve (pos, n);
        pos = begin () + at;
        for (int i = 0; i < n; ++i)
          pos[i] = t;
        _last += n;
      }

    return pos;
  }

  void erase (iterator first, iterator last)
  {
    if (last != first)
      {
        memmove (first, last, (end () - last) * sizeof (T));
        _last -= last - first;
      }
  }

  void erase (iterator pos)
  {
    if (pos != end ())
      {
        memmove (pos, pos + 1, (end () - (pos + 1)) * sizeof (T));
        --_last;
      }
  }

  void swap (simplevec<T> &t)
  {
    ::swap(_last, t._last);
    ::swap(_size, t._size);
    ::swap(_buf, t._buf);
  }
};

template<class T>
bool operator ==(const simplevec<T> &v1, const simplevec<T> &v2)
{
  if (v1.size () != v2.size ()) return false;

  return !v1.size () || !memcmp (&v1[0], &v2[0], v1.size () * sizeof (T));
}

template<class T>
bool operator <(const simplevec<T> &v1, const simplevec<T> &v2)
{
  unsigned long minlast = min (v1.size (), v2.size ());

  for (unsigned long i = 0; i < minlast; ++i)
    {
      if (v1[i] < v2[i]) return true;
      if (v2[i] < v1[i]) return false;
    }
  return v1.size () < v2.size ();
}

template<typename T>
struct vector : simplevec<T>
{
};

#endif
