#ifndef GEOMTETRY_2D_POINT_H
#define GEOMTETRY_2D_POINT_H

#include <iostream>

namespace neutu {

namespace geom2d {

class Point
{
public:
  Point();
  Point(double x, double y);

  void set(double x, double y);
  void setX(double x);
  void setY(double y);

  double getX() const;
  double getY() const;

  bool isValid() const;

  static Point InvalidPoint();

  bool operator== (const Point &pt) const;
  bool operator!= (const Point &pt) const;

  friend std::ostream& operator << (
        std::ostream &stream, const neutu::geom2d::Point &pt);
private:
  double m_x = 0.0;
  double m_y = 0.0;
};

}

}

#endif // GEOMTETRY_2D_POINT_H
