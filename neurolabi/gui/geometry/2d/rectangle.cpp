#include "rectangle.h"

#include "common/utilities.h"

neutu::geom2d::Rectangle::Rectangle()
{

}

neutu::geom2d::Rectangle::Rectangle(double x0, double y0, double x1, double y1)
{
  set(x0, y0, x1, y1);
}

neutu::geom2d::Rectangle::Rectangle(
    const Point &minCorner, const Point &maxCorner)
{
  set(minCorner.getX(), minCorner.getY(), maxCorner.getX(), maxCorner.getY());
}

void neutu::geom2d::Rectangle::set(double x0, double y0, double x1, double y1)
{
  setMinCorner(x0, y0);
  setMaxCorner(x1, y1);
}

void neutu::geom2d::Rectangle::set(double x0, double y0, const Dims &dims)
{
  set(x0, y0, x0 + dims.getWidth(), y0 + dims.getHeight());
}

void neutu::geom2d::Rectangle::setCenter(
    double x, double y, double width, double height)
{
  set(x - width / 2.0, y - height / 2.0, x + width / 2.0, y + height / 2.0);
}

void neutu::geom2d::Rectangle::setMinCorner(double x, double y)
{
  m_x0 = x;
  m_y0 = y;
}

void neutu::geom2d::Rectangle::setMaxCorner(double x, double y)
{
  m_x1 = x;
  m_y1 = y;
}

void neutu::geom2d::Rectangle::setMinCorner(const Point &pt)
{
  setMinCorner(pt.getX(), pt.getY());
}

void neutu::geom2d::Rectangle::setMaxCorner(const Point &pt)
{
  setMaxCorner(pt.getX(), pt.getY());
}

double neutu::geom2d::Rectangle::getWidth() const
{
  return m_x1 - m_x0;
}
double neutu::geom2d::Rectangle::getHeight() const
{
  return m_y1 - m_y0;
}

neutu::geom2d::Point neutu::geom2d::Rectangle::getMinCorner() const
{
  return Point(m_x0, m_y0);
}

neutu::geom2d::Point neutu::geom2d::Rectangle::getMaxCorner() const
{
  return Point(m_x1, m_y1);
}

neutu::geom2d::Point neutu::geom2d::Rectangle::getCorner(int index) const
{
  switch (index) {
  case 0:
    return Point(m_x0, m_y0);
  case 1:
    return Point(m_x1, m_y0);
  case 2:
    return Point(m_x0, m_y1);
  case 3:
    return Point(m_x1, m_y1);
  }

  return Point::InvalidPoint();
}

neutu::geom2d::Point neutu::geom2d::Rectangle::getCenter() const
{
  return Point((m_x0 + m_x1) / 2.0, (m_y0 + m_y1) / 2.0);
}

bool neutu::geom2d::Rectangle::contains(const Point &pt) const
{
  return neutu::WithinCloseRange(pt.getX(), m_x0, m_x1) &&
      neutu::WithinCloseRange(pt.getY(), m_y0, m_y1);
}

bool neutu::geom2d::Rectangle::contains(const Rectangle &rect) const
{
  for (int i = 0; i < 3; ++i) {
    if (!contains(rect.getCorner(i))) {
      return false;
    }
  }

  return true;
}

bool neutu::geom2d::Rectangle::intersecting(const Rectangle &rect) const
{
  for (int i = 0; i < 3; ++i) {
    if (contains(rect.getCorner(i))) {
      return true;
    }

    if (rect.contains(getCorner(i))) {
      return true;
    }
  }

  return false;
}

void neutu::geom2d::Rectangle::round()
{
  m_x0 = std::round(m_x0);
  m_y0 = std::round(m_y0);
  m_x1 = std::round(m_x1);
  m_y1 = std::round(m_y1);
}

bool neutu::geom2d::Rectangle::operator== (const Rectangle &r) const
{
  return (m_x0 == r.m_x0) && (m_x1 == r.m_x1) && (m_y0 == r.m_y0) && (m_y1 == r.m_y1);
}

bool neutu::geom2d::Rectangle::operator!= (const Rectangle &r) const
{
  return (m_x0 != r.m_x0) || (m_x1 != r.m_x1) || (m_y0 != r.m_y0) || (m_y1 != r.m_y1);
}

bool neutu::geom2d::Rectangle::isValid() const
{
  return getWidth() > 0.0 && getHeight() > 0.0;
}

std::ostream& operator << (
    std::ostream &stream, const neutu::geom2d::Rectangle &r)
{
  stream << r.getMinCorner() << "->" << r.getMaxCorner();

  return stream;
}
