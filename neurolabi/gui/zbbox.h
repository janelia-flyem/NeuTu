#ifndef ZBBOX_H
#define ZBBOX_H

#include <iostream>
#include <limits>

template<typename Point>
class ZBBox
{
public:
  // inverse box
  ZBBox()
  { reset(); }

  explicit ZBBox(const Point& minmaxCorner)
    : m_minCorner(minmaxCorner), m_maxCorner(minmaxCorner)
  {}

  ZBBox(const Point& minCorner, const Point& maxCorner)
    : m_minCorner(minCorner), m_maxCorner(maxCorner)
  {}

  ZBBox(ZBBox&&) noexcept = default;

  ZBBox& operator=(ZBBox&&) noexcept = default;

  ZBBox(const ZBBox&) = default;

  ZBBox& operator=(const ZBBox&) = default;

  inline void reset()
  {
    for (size_t i = 0; i < m_minCorner.length(); ++i) {
      m_minCorner[i] = std::numeric_limits<typename Point::value_type>::max();
      m_maxCorner[i] = std::numeric_limits<typename Point::value_type>::lowest();
    }
  }

  inline Point const& minCorner() const
  { return m_minCorner; }

  inline Point const& maxCorner() const
  { return m_maxCorner; }

  inline void setMinCorner(const Point& mc)
  { m_minCorner = mc; }

  inline void setMaxCorner(const Point& mc)
  { m_maxCorner = mc; }

  //
  inline bool empty() const
  {
    for (size_t i = 0; i < m_minCorner.length(); ++i) {
      if (m_minCorner[i] > m_maxCorner[i])
        return true;
    }
    return false;
  }

  //
  inline Point size() const
  { return m_maxCorner - m_minCorner; }

  //
  inline typename Point::value_type volume() const
  {
    Point sz = size();
    typename Point::value_type res = 1;
    for (size_t i = 0; i < sz.length(); ++i) {
      res *= sz[i];
    }
    return res;
  }

  //comparison
  inline bool operator==(const ZBBox& other)
  { return m_minCorner == other.m_minCorner && m_maxCorner == other.m_maxCorner; }

  inline bool operator!=(const ZBBox& other)
  { return m_minCorner != other.m_minCorner || m_maxCorner != other.m_maxCorner; }

  //expand to contain
  inline void expand(const ZBBox& other)
  {
    m_minCorner = min(m_minCorner, other.m_minCorner);
    m_maxCorner = max(m_maxCorner, other.m_maxCorner);
  }

  inline void expand(const Point& other)
  {
    m_minCorner = min(m_minCorner, other);
    m_maxCorner = max(m_maxCorner, other);
  }

  inline void expand(typename Point::value_type v)
  {
    m_minCorner -= v;
    m_maxCorner += v;
  }

  inline ZBBox& operator+=(const ZBBox& other)
  {
    expand(other);
    return *this;
  }

  inline ZBBox& operator+=(const Point& other)
  {
    expand(other);
    return *this;
  }

  //intersect
  inline ZBBox& intersect(const ZBBox& other)
  {
    m_minCorner = max(m_minCorner, other.m_minCorner);
    m_maxCorner = min(m_maxCorner, other.m_maxCorner);
    return *this;
  }

  //
  inline bool contains(const ZBBox& other) const
  {
    for (size_t i = 0; i < m_minCorner.length(); ++i) {
      if (other.m_minCorner[i] < m_minCorner[i] || other.m_maxCorner[i] > m_maxCorner[i])
        return false;
    }
    return true;
  }

  inline bool contains(const Point& other) const
  {
    for (size_t i = 0; i < m_minCorner.length(); ++i) {
      if (other[i] < m_minCorner[i] || other[i] > m_maxCorner[i])
        return false;
    }
    return true;
  }

  inline bool containedBy(const ZBBox& other) const
  { return other.contains(*this); }

  // tests if bounding boxes (and points) are disjoint (empty intersection)
  inline bool disjoint(const ZBBox& other) const
  {
    const Point sz = min(other.m_maxCorner, m_maxCorner) - max(other.m_minCorner, m_minCorner);
    for (size_t i = 0; i < sz.length(); ++i) {
      if (sz[i] < 0)
        return true;
    }
    return false;
  }

  inline bool disjoint(const Point& other) const
  {
    const Point sz = min(other, m_maxCorner) - max(other, m_minCorner);
    for (size_t i = 0; i < sz.length(); ++i) {
      if (sz[i] < 0)
        return true;
    }
    return false;
  }

  // tests if bounding boxes (and points) are conjoint (non-empty intersection)
  inline bool conjoint(const ZBBox& other) const
  { return !disjoint(other); }

  inline bool conjoint(const Point& other) const
  { return !disjoint(other); }

private:
  Point m_minCorner;
  Point m_maxCorner;
};

// merges bounding boxes and points, same as expand
template<typename T>
inline ZBBox<T> merge(const ZBBox<T>& a, const T& b)
{ return ZBBox<T>(min(a.minCorner(), b), max(a.maxCorner(), b)); }

template<typename T>
inline ZBBox<T> merge(const ZBBox<T>& a, const ZBBox<T>& b)
{ return ZBBox<T>(min(a.minCorner(), b.minCorner()), max(a.maxCorner(), b.maxCorner())); }

template<typename T>
inline ZBBox<T> merge(const ZBBox<T>& a, const ZBBox<T>& b, const ZBBox<T>& c)
{ return merge(a, merge(b, c)); }

//
template<typename T>
inline ZBBox<T> intersect(const ZBBox<T>& a, const ZBBox<T>& b)
{ return ZBBox<T>(max(a.minCorner(), b.minCorner()), min(a.maxCorner(), b.maxCorner())); }

template<typename T>
inline ZBBox<T> intersect(const ZBBox<T>& a, const ZBBox<T>& b, const ZBBox<T>& c)
{ return intersect(a, intersect(b, c)); }

//
template<typename T>
inline bool disjoint(const ZBBox<T>& a, const ZBBox<T>& b)
{ return a.disjoint(b); }

template<typename T>
inline bool disjoint(const ZBBox<T>& a, const T& b)
{ return a.disjoint(b); }

template<typename T>
inline bool disjoint(const T& a, const ZBBox<T>& b)
{ return b.disjoint(a); }

//
template<typename T>
inline bool conjoint(const ZBBox<T>& a, const ZBBox<T>& b)
{ return a.conjoint(b); }

template<typename T>
inline bool conjoint(const ZBBox<T>& a, const T& b)
{ return a.conjoint(b); }

template<typename T>
inline bool conjoint(const T& a, const ZBBox<T>& b)
{ return b.conjoint(a); }

//
template<typename T>
inline std::ostream& operator<<(std::ostream& cout, const ZBBox<T>& box)
{
  return cout << "[" << box.minCorner() << "; " << box.maxCorner() << "]";
}

#endif // ZBBOX_H
