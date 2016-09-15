#ifndef ZSTACKVIEWPARAM_H
#define ZSTACKVIEWPARAM_H

#include <QRect>
#include "neutube_def.h"

/*!
 * \brief The class of stack view parameter
 */
class ZStackViewParam
{
public:
  ZStackViewParam();
  ZStackViewParam(NeuTube::ECoordinateSystem coordSys);

  inline NeuTube::ECoordinateSystem getCoordinateSystem() const {
    return m_coordSys;
  }

  inline int getZ() const {
    return m_z;
  }

  int getArea() const;

  inline const QRect& getViewPort() const {
    return m_viewPort;
  }

  inline NeuTube::View::EExploreAction getExploreAction() const {
    return m_action;
  }


  inline const QRectF& getProjRect() const {
    return m_projRect;
  }

  void setZ(int z);
  void setViewPort(const QRect &rect);
  void setProjRect(const QRectF &rect);

  double getZoomRatio() const;

  void setViewPort(double x0, double y0, double x1, double y1);
  void setExploreAction(NeuTube::View::EExploreAction action);
  void setSliceAxis(NeuTube::EAxis sliceAxis);
  NeuTube::EAxis getSliceAxis() const;

  bool operator ==(const ZStackViewParam &param) const;
  bool operator !=(const ZStackViewParam &param) const;

  bool contains(const ZStackViewParam &param) const;

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

private:
  void init(NeuTube::ECoordinateSystem coordSys);

private:
  int m_z;
  QRect m_viewPort;
  QRectF m_projRect;
  NeuTube::ECoordinateSystem m_coordSys;
  NeuTube::View::EExploreAction m_action;
  NeuTube::EAxis m_sliceAxis;
  bool m_fixingZ;
};

#endif // ZSTACKVIEWPARAM_H
