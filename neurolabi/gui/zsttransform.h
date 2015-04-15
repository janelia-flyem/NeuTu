#ifndef ZSTTRANSFORM_H
#define ZSTTRANSFORM_H


#include "neutube.h"

class ZPoint;
class QPointF;
class QPoint;
class QRectF;

/*!
 * \brief The ZStTransform class
 *
 * Scale-translate transformation: Y = sX + t
 */
class ZStTransform
{
public:
  ZStTransform();

  void setOffset(double dx, double dy, double dz);
  void setScale(double sx, double sy, double sz);

  void setOffset(double dx, double dy);
  void setScale(double sx, double sy);

  inline double getSx() const { return m_sx; }
  inline double getSy() const { return m_sy; }
  inline double getSz() const { return m_sz; }
  inline double getTx() const { return m_dx; }
  inline double getTy() const { return m_dy; }
  inline double getTz() const { return m_dz; }

  inline double transformX(double x) const {
    return x * m_sx + m_dx;
  }

  inline double transformY(double y) const {
    return y * m_sy + m_dy;
  }

  inline double transformZ(double z) const {
    return z * m_sz + m_dz;
  }

  double getScale(NeuTube::EAxis axis) const;
  double getOffset(NeuTube::EAxis axis) const;
  ZPoint getOffset() const;

  bool isIntTransform() const;
  bool isIntOffset() const;
  bool isIntScale() const;
  bool hasOffset() const;
  bool hasScale() const;

  ZStTransform getInverseTransform() const;
  ZPoint transform(const ZPoint &pt) const;
  QPointF transform(const QPointF &pt) const;
  QPointF transform(const QPoint &pt) const;
  QRectF transform(const QRectF &rect) const;

  bool isIdentity() const;

private:
  double m_sx;
  double m_sy;
  double m_sz;
  double m_dx;
  double m_dy;
  double m_dz;
};

#endif // ZSTTRANSFORM_H
