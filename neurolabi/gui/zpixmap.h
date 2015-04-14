#ifndef ZPIXMAP_H
#define ZPIXMAP_H

#include <QPixmap>

#include "zsttransform.h"

class ZPixmap : public QPixmap
{
public:
  ZPixmap();
  ZPixmap(const QSize &size);

  const ZStTransform& getTransform() const;
  void setScale(double sx, double sy);
  void setOffset(double dx, double dy);

private:
  ZStTransform m_transform;
};

#endif // ZPIXMAP_H
