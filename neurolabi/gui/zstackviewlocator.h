#ifndef ZSTACKVIEWLOCATOR_H
#define ZSTACKVIEWLOCATOR_H

#include <QRect>
#include <QSize>

class ZStackViewLocator
{
public:
  ZStackViewLocator();

  void setSceneRatio(double ratio);

  void setCanvasSize(int w, int h);

  QRect getLandmarkViewPort(double cx, double cy, double radius) const;
  QRect getRectViewPort(double cx, double cy, double viewWidth) const;

  int getZoomRatio(int viewPortWidth, int viewPortHeight) const;
  inline double getSceneRatio() const { return m_sceneRatio; }

private:
  double m_sceneRatio; //0 to 1
  QSize m_canvasSize;
  int m_minZoomRatio;
};

#endif // ZSTACKVIEWLOCATOR_H
