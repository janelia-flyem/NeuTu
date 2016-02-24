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

  void setZ(int z);
  void setViewPort(const QRect &rect);
  void setViewPort(double x0, double y0, double x1, double y1);
  void setExploreAction(NeuTube::View::EExploreAction action);
  void setSliceAxis(NeuTube::EAxis sliceAxis);
  NeuTube::EAxis getSliceAxis() const;

  bool operator ==(const ZStackViewParam &param) const;
  bool operator !=(const ZStackViewParam &param) const;

  bool contains(const ZStackViewParam &param) const;

  /*!
   * \brief Resize the parameter by keeping the center relatively constant
   */
  void resize(int width, int height);

private:
  void init(NeuTube::ECoordinateSystem coordSys);

private:
  int m_z;
  QRect m_viewPort;
  NeuTube::ECoordinateSystem m_coordSys;
  NeuTube::View::EExploreAction m_action;
  NeuTube::EAxis m_sliceAxis;
};

#endif // ZSTACKVIEWPARAM_H
