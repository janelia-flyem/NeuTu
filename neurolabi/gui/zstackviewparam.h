#ifndef ZSTACKVIEWPARAM_H
#define ZSTACKVIEWPARAM_H

#include <QRect>
#include "neutube.h"

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

  inline const QRect& getViewPort() const {
    return m_viewPort;
  }

  inline NeuTube::View::EExploreAction getExploreAction() const {
    return m_action;
  }

  void setZ(int z);
  void setViewPort(const QRect &rect);
  void setViewPort(int x0, int y0, int x1, int y1);
  void setExploreAction(NeuTube::View::EExploreAction action);

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
};

#endif // ZSTACKVIEWPARAM_H
