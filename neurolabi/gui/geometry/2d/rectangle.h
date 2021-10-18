#ifndef GEOMTETRY_2D_RECTANGLE_H
#define GEOMTETRY_2D_RECTANGLE_H

#include "point.h"
#include "dims.h"

namespace neutu {

namespace geom2d {

class Rectangle
{
public:
  Rectangle();
  Rectangle(double x0, double y0, double x1, double y1);
  Rectangle(const Point &minCorner, const Point &maxCorner);

  void set(double x0, double y0, double x1, double y1);
  void set(double x0, double y0, const Dims &dims);
  void setCenter(double x, double y, double width, double height);
  void setMinCorner(double x, double y);
  void setMaxCorner(double x, double y);
  void setMinCorner(const Point &pt);
  void setMaxCorner(const Point &pt);

  double getWidth() const;
  double getHeight() const;
  Point getMinCorner() const;
  Point getMaxCorner() const;
  Point getCorner(int index) const;
  Point getCenter() const;

  bool contains(const Point &pt) const;
  bool contains(const Rectangle &rect) const;

  bool intersecting(const Rectangle &rect) const;

  /*!
   * \brief A rectangle is valid iff its dims are positive.
   */
  bool isValid() const;

  /*!
   * \brief Round the corner coorindates
   */
  void round();

  bool operator== (const Rectangle &r) const;
  bool operator!= (const Rectangle &r) const;

private:
  double m_x0 = 0.0;
  double m_y0 = 0.0;
  double m_x1 = 0.0;
  double m_y1 = 0.0;
};

}

}

std::ostream& operator << (
      std::ostream &stream, const neutu::geom2d::Rectangle &r);


#endif // RECTANGLE_H
