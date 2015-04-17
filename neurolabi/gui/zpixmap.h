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

  void clearnUp();

  inline bool isVisible() const {
    return m_isVisible;
  }

  void setVisible(bool visible) {
    m_isVisible = visible;
  }

private:
  ZStTransform m_transform;
  bool m_isVisible;
};

#endif // ZPIXMAP_H
