#ifndef ZSTACKVIEWPARAM_H
#define ZSTACKVIEWPARAM_H

#include <QRect>
#include "neutube_def.h"

#include "zviewproj.h"

/*!
 * \brief The class of stack view parameter
 */
class ZStackViewParam
{
public:
  ZStackViewParam();
  ZStackViewParam(neutube::ECoordinateSystem coordSys);

  inline neutube::ECoordinateSystem getCoordinateSystem() const {
    return m_coordSys;
  }

  inline int getZ() const {
    return m_z;
  }

  bool isValid() const;

  int getArea() const;

  QRect getViewPort() const;
  QRectF getProjRect() const;

  inline neutube::View::EExploreAction getExploreAction() const {
    return m_action;
  }

  void setZOffset(int z0) {
    m_z0 = z0;
  }
  int getSliceIndex() const;
  void setSliceIndex(int index);


  void setZ(int z);
  void setViewProj(const ZViewProj &vp);


//  void setProjRect(const QRectF &rect);

  double getZoomRatio() const;

  void setWidgetRect(const QRect &rect);
  void setCanvasRect(const QRect &rect);

  void setViewPort(double x0, double y0, double x1, double y1);
  void setViewPort(const QRect &rect);
  void setExploreAction(neutube::View::EExploreAction action);
  void setSliceAxis(neutube::EAxis sliceAxis);
  neutube::EAxis getSliceAxis() const;

  bool operator ==(const ZStackViewParam &param) const;
  bool operator !=(const ZStackViewParam &param) const;

  bool contains(const ZStackViewParam &param) const;
  bool containsViewport(const ZStackViewParam &param) const;

  bool contains(int x, int y, int z);

  /*!
   * \brief Resize the parameter by keeping the center relatively constant
   */
  void resize(int width, int height);

  void fixZ(bool state) {
    m_fixingZ = state;
  }

  bool fixingZ() const {
    return m_fixingZ;
  }

  int getZoomLevel(int maxLevel) const;

  const ZViewProj& getViewProj() const {
    return m_viewProj;
  }


private:
  void init(neutube::ECoordinateSystem coordSys);

private:
  int m_z;
  ZViewProj m_viewProj;
  int m_z0 = 0;
  neutube::ECoordinateSystem m_coordSys;
  neutube::View::EExploreAction m_action;
  neutube::EAxis m_sliceAxis;
  bool m_fixingZ;
};

#endif // ZSTACKVIEWPARAM_H
