#ifndef ZDVIDGRAYSLICE_H
#define ZDVIDGRAYSLICE_H

#include "zstackobject.h"
#include "zimage.h"
#include "zdvidtarget.h"

class ZRect2d;

class ZDvidGraySlice : public ZStackObject
{
public:
  ZDvidGraySlice();
  ~ZDvidGraySlice();

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;
  void clear();

  void update(int z);

  void loadDvidSlice(const QByteArray &buffer, int z);

  virtual const std::string& className() const;

  void printInfo() const;

  void setDvidTarget(const ZDvidTarget &target);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  inline int getX() const {
    return m_x;
  }
  inline int getY() const {
    return m_y;
  }
  inline int getZ() const {
    return m_z;
  }
  inline void setZ(int z) {
    m_z = z;
  }

  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

  ZRect2d getBoundBox() const;
  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

  void setBoundBox(const ZRect2d &rect);

private:
  void updateImage();

  /*!
   * \brief Check if the regions of the image and the slice are consistent.
   */
  bool isRegionConsistent() const;

private:
  ZImage m_image;
  int m_x;
  int m_y;
  int m_z;
  int m_width;
  int m_height;

  ZDvidTarget m_dvidTarget;
};

#endif // ZDVIDGRAYSLICE_H
