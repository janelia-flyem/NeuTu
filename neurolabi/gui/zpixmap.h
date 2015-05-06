#ifndef ZPIXMAP_H
#define ZPIXMAP_H

#include <QPixmap>
#include <QRect>

#include "zsttransform.h"
#include "neutube.h"

class ZPixmap : public QPixmap
{
public:
  ZPixmap();
  ZPixmap(const QSize &size);
  ZPixmap(int width, int height);

  const ZStTransform& getTransform() const;
  void setScale(double sx, double sy);
  void setOffset(double dx, double dy);

  void cleanUp();
  void clean(const QRect &rect);


  inline bool isVisible() const {
    return m_isVisible;
  }

  void setVisible(bool visible) {
    m_isVisible = visible;
  }

  QRectF getActiveArea(NeuTube::ECoordinateSystem coord) const;
  bool isFullyActive() const;

private:
  ZStTransform m_transform; //Transformation from world coordinates to image coordinates
  QRectF m_activeArea; //Active area in the world coordiantes
  bool m_isVisible;

  QPixmap m_cleanBuffer;
};

#endif // ZPIXMAP_H
