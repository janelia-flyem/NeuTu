#ifndef ZIMAGEFRAME_H
#define ZIMAGEFRAME_H

#include <QImage>

class ZImageFrame
{
public:
  ZImageFrame();

  const QImage& getImage() const {
    return m_image;
  }

private:
  QImage m_image;
};

#endif // ZIMAGEFRAME_H
