#ifndef GEOMTETRY_2D_DIMS_H
#define GEOMTETRY_2D_DIMS_H

namespace neutu {

namespace geom2d {

class Dims
{
public:
  Dims();
  Dims(double w, double h);

  double getWidth() const;
  double getHeight() const;

  void set(double w, double h);
  void setWidth(double w);
  void setHeight(double h);

private:
  double m_width = 0.0;
  double m_height = 0.0;
};

}

}

#endif // GEOMTETRY_2D_DIMS_H
