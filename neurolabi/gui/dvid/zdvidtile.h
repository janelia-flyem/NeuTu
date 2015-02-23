#ifndef ZDVIDTILE_H
#define ZDVIDTILE_H

#include <QImage>
#include "zstackobject.h"
#include "dvid/zdvidresolution.h"
#include "zdvidtarget.h"
#include "zintpoint.h"

class ZPainter;
class ZStack;
class ZDvidResolution;

class ZDvidTile : public ZStackObject
{
public:
  ZDvidTile();
  ~ZDvidTile();

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option) const;
  void clear();

  void update(int x, int y, int z, int width, int height);

  void loadDvidPng(const QByteArray &buffer);
  void setResolutionLevel(int level);
  void setTileOffset(int x, int y, int z);

  virtual const std::string& className() const;

  void printInfo() const;

private:
  QImage m_image;
  ZIntPoint m_offset;
  ZDvidResolution m_res;
  ZDvidTarget m_dvidTarget;
};

#endif // ZDVIDTILE_H
