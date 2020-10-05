#include "point.h"

#include <limits>
#include <cmath>

neutu::geom2d::Point::Point()
{
}

neutu::geom2d::Point::Point(double x, double y) :
  m_x(x), m_y(y)
{
}

void neutu::geom2d::Point::set(double x, double y)
{
  m_x = x;
  m_y = y;
}

void neutu::geom2d::Point::setX(double x)
{
  m_x = x;
}

void neutu::geom2d::Point::setY(double y)
{
  m_y = y;
}

double neutu::geom2d::Point::getX() const
{
  return m_x;
}

double neutu::geom2d::Point::getY() const
{
  return m_y;
}

bool neutu::geom2d::Point::isValid() const
{
  return !(std::isnan(m_x) || std::isnan(m_y));
}

bool neutu::geom2d::Point::operator== (const Point &pt) const
{
  return (m_x == pt.m_x) && (m_y == pt.m_y);
}

bool neutu::geom2d::Point::operator!= (const Point &pt) const
{
  return (m_x != pt.m_x) || (m_y != pt.m_y);
}

neutu::geom2d::Point neutu::geom2d::Point::InvalidPoint()
{
  return Point(std::numeric_limits<double>::quiet_NaN(),
               std::numeric_limits<double>::quiet_NaN());
}

std::ostream& operator << (
    std::ostream &stream, const neutu::geom2d::Point &pt)
{
  return stream << "(" << pt.getX() << ", " << pt.getY() << ")";
}
