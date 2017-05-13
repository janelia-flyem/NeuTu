#ifndef ZDVIDGRAYSCALE_H
#define ZDVIDGRAYSCALE_H

#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>

#include "zstackobject.h"
#include "zdvidreader.h"
#include "zpixmap.h"
#include "zjsonobject.h"

class ZPainter;
class ZStack;
class ZStackView;
class ZRect2d;
class ZIntPoint;
class ZImage;

/*!
 * \brief Obsolete.
 */
class ZDvidGrayscale
{
public:
  ZDvidGrayscale();
  ~ZDvidGrayscale();


private:
  ZDvidReader m_reader;
  ZImage *m_image;
  ZPixmap m_pixmap;

  ZIntPoint m_offset;
  int m_width;
  int m_height;
  int m_zoom;
  ZJsonObject m_contrastProtocal;

  ZStackView *m_view;

  QMutex m_pixmapMutex;
};

#endif // ZDVIDGRAYSCALE_H
