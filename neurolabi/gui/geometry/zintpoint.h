#ifndef ZINTPOINT_H
#define ZINTPOINT_H

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>

#include <boost/functional/hash.hpp>

#include "common/neutudefs.h"

class ZPoint;

/*!
 * \brief The class of 3D points with integer coordinates
 *
 * The point (INT_MIN, INT_MIN, INT_MIN) is reserved for invalid point.
 */
class ZIntPoint
{
public:
  ZIntPoint();
  ZIntPoint(int x, int y, int z);

  inline int getX() const { return m_x; }
  inline int getY() const { return m_y; }
  inline int getZ() const { return m_z; }

  void set(int x, int y, int z);
  /*!
   * \brief Set a point from an array.
   *
   * Nothing is done if the size of \a pt is not 3.
   */
  void set(const std::vector<int> &pt);

  inline void setX(int x) { m_x = x; }
  inline void setY(int y) { m_y = y; }
  inline void setZ(int z) { m_z = z; }

  int getValue(neutu::EAxis axis) const;
  void setValue(int v, neutu::EAxis axis);

  const int& operator[] (int index) const;
  int& operator[] (int index);

  /*!
   * \brief Comparison.
   *
   * The result of comparing with an invalid point is undefined.
   *
   * Compare order: z, y, x
   */
  bool operator < (const ZIntPoint &pt) const;

  /*!
   * \brief Check if a point is defintely less than another point
   *
   * \return true iff any of the coordinate value than that of \a pt and the
   * other two coordinate values are not greater than their counterparts of \a pt.
   */
  bool definiteLessThan(const ZIntPoint &pt) const;

  bool definitePositive() const;
  bool semiDefinitePositive() const;

  /*!
   * \brief Check if tow points are the same.
   *
   * Tow invalid points are considered as the same.
   */
  bool operator == (const ZIntPoint &pt) const;

  /*!
   * \brief Check if tow points are different.
   */
  bool operator != (const ZIntPoint &pt) const;


  /**@addtogroup _opr Point operations
   *
   * Note: An operation always results in an invalid point if any of the inputs
   * is invalid.
   *
   * @{
   */

  /*!
   * \brief Negate a point.
   */
  ZIntPoint operator - () const;

  ZIntPoint& operator += (const ZIntPoint &pt);
  ZIntPoint& operator -= (const ZIntPoint &pt);
  ZIntPoint& operator *= (const ZIntPoint &pt);
  ZIntPoint& operator /= (const ZIntPoint &pt);
  ZIntPoint& operator /= (int v);

  friend ZIntPoint operator + (const ZIntPoint &pt1, const ZIntPoint &pt2);
  friend ZIntPoint operator + (const ZIntPoint &pt1, int v);
  friend ZIntPoint operator * (const ZIntPoint &pt1, const ZIntPoint &pt2);
  friend ZIntPoint operator * (const ZIntPoint &pt1, int v);
  friend ZIntPoint operator - (const ZIntPoint &pt1, const ZIntPoint &pt2);
  friend ZIntPoint operator - (const ZIntPoint &pt1, int v);

  /*!
   * \brief Coordinate-wise division
   *
   * It returns (0, 0, 0) if \a pt2 has a 0 coordinate value.
   */
  friend ZIntPoint operator / (const ZIntPoint &pt1, const ZIntPoint &pt2);
  friend ZIntPoint operator / (const ZIntPoint &pt1, int scale);

  /**@}*/

  std::string toString() const;

  /*!
   * \brief Convert the point to a string according to a template
   *
   * Any "$x"/"$X", "$y"/"$Y", "$z"/"$Z" substring in the template will be
   * replaced by the X, Y, Z coordiates of the point respectively. An empty
   * template results in a normal point string, which is the same from
   * \a toString().
   */
  std::string toString(const std::string &templ) const;

  ZPoint toPoint() const;
  ZPoint toMinCorner() const;
  ZPoint toMaxCorner() const;

  /*!
   * \brief Test if the coordinates are (0, 0, 0)
   */
  bool isZero() const;

  bool equals(const ZIntPoint &pt) const;

  double distanceSquareTo(double x, double y, double z) const;
  double distanceTo(double x, double y, double z) const;

  void shiftSliceAxis(neutu::EAxis axis);
  void shiftSliceAxisInverse(neutu::EAxis axis);
  int getCoord(neutu::EAxis axis) const;

  void invalidate();
  bool isValid() const;
  static bool IsValid(int x);

  static bool IsNormalDimIndex(int index);

  void read(std::istream &stream);
  void write(std::ostream &stream) const;

  friend std::ostream& operator<<(std::ostream& stream, const ZIntPoint &pt);

public:
  int m_x;
  int m_y;
  int m_z;
};


namespace std
{
template<> struct hash<ZIntPoint>
{
  typedef std::size_t result_type;
  result_type operator()(ZIntPoint const& s) const noexcept
  {
    result_type seed = 0;

    boost::hash_combine(seed, s.getX());
    boost::hash_combine(seed, s.getY());
    boost::hash_combine(seed, s.getZ());

    return seed;
  }
};
}

#endif // ZINTPOINT_H
