#ifndef ZVIEWPLANETRANSFORM_H
#define ZVIEWPLANETRANSFORM_H

//class QTransform;

/*!
 * \brief Aspect-ratio-preserved scaling-translation transform
 *
 * xs = xv * s + dx
 */
class ZViewPlaneTransform
{
public:
  ZViewPlaneTransform();

  void set(double dx, double dy, double s);
  double getScale() const;
  double getTx() const;
  double getTy() const;

//  QTransform getPainterTransform() const;

private:
  double m_dx = 0.0;
  double m_dy = 0.0;
  double m_s = 1.0;
};

#endif // ZVIEWPLANETRANSFORM_H
