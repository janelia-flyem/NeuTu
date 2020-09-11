#ifndef ZIMAGEFRAMESHOT_H
#define ZIMAGEFRAMESHOT_H

#include <memory>

#include <QImage>
#include <QRect>

class ZStackSource;

class ZImageFrameShot
{
public:
  ZImageFrameShot(int w, int h);

  QImage takeShot(const QRect &viewport, int z);

  void setStackSource(std::shared_ptr<ZStackSource> source);

private:
  std::shared_ptr<ZStackSource> m_stackSource;
  int m_destWidth;
  int m_destHeight;
};

#endif // ZIMAGEFRAMESHOT_H
