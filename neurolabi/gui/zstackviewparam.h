#ifndef ZSTACKVIEWPARAM_H
#define ZSTACKVIEWPARAM_H

#include <QRect>
#include "common/neutudefs.h"

//#include "zviewproj.h"
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"
#include "data3d/zsliceviewtransform.h"

class ZJsonObject;
class ZArbSliceViewParam;
//class ZAffinePlane;
//class ZAffineRect;

/*!
 * \brief The class of stack view parameter
 */
class ZStackViewParam
{
public:
  ZStackViewParam();
  ZStackViewParam(const ZSliceViewTransform &t, int width, int height,
                  neutu::data3d::ESpace sizeSpace);

  void set(const ZSliceViewTransform &t, int width, int height,
           neutu::data3d::ESpace sizeSpace);
  void setTransform(const ZSliceViewTransform &t);

  ZAffineRect getCutRect() const;
  /*!
   * \brief Get a rectangle adjusted for the integer space
   *
   * The result uses the right integer center.
   */
  ZAffineRect getIntCutRect() const;

  ZAffineRect getIntCutRect(const ZIntCuboid &modelRange) const;

  double getWidth(neutu::data3d::ESpace space) const;
  double getHeight(neutu::data3d::ESpace space) const;

  int getIntWidth(neutu::data3d::ESpace space) const;
  int getIntHeight(neutu::data3d::ESpace space) const;

  /*!
   * \brief Convert the values in the model space into integer values
   *
   * The values that become integers including the cut center and the size in
   * the model space.
   */
//  void discretizeModel();

//  ZStackViewParam getDiscretized() const;
  ZArbSliceViewParam toArbSliceViewParam() const;

  /*
  inline neutu::ECoordinateSystem getCoordinateSystem() const {
    return m_coordSys;
  }

  inline int getZ() const {
    return m_z;
  }
  */

  /*!
   * \brief Test if the viewport is empty
   */
  bool isViewportEmpty() const;

  /*!
   * \brief Test if the viewport is valid (non-empty)
   */
  bool isValid() const;
  void invalidate();

  double getArea(neutu::data3d::ESpace space) const;

//  QRect getViewPort() const;
//  QRectF getProjRect() const;

  /*
  inline neutu::View::EExploreAction getExploreAction() const {
    return m_action;
  }
  */

  /*
  void setZOffset(int z0) {
    m_z0 = z0;
  }

  int getSliceIndex() const;
  void setSliceIndex(int index);


  void setZ(int z);
  void setViewProj(const ZViewProj &vp);
*/

//  void setProjRect(const QRectF &rect);

  double getZoomRatio() const;
  int getZoomLevel(int maxLevel) const;
  int getZoomLevel() const;

//  void setWidgetRect(const QRect &rect);
//  void setCanvasRect(const QRect &rect);

//  void setViewPort(double x0, double y0, double x1, double y1);
//  void setViewPort(const QRect &rect);
//  void setViewPort(const QRect &rect, int z);
  void closeViewPort();
  void openViewPort();

//  void setExploreAction(neutu::View::EExploreAction action);
  void setSliceAxis(neutu::EAxis sliceAxis);
  neutu::EAxis getSliceAxis() const;

  bool operator ==(const ZStackViewParam &param) const;
  bool operator !=(const ZStackViewParam &param) const;

  bool contains(const ZStackViewParam &param) const;
  bool contains(double x, double y, double z) const;
  bool contains(const ZPoint &pt) const;

  /*!
   * \brief Resize the parameter by keeping the center relatively constant
   */
  void setSize(int width, int height, neutu::data3d::ESpace sizeSpace);

  ZPoint getCutCenter() const;

  void setCutCenter(const ZIntPoint &pt);
  void setCutDepth(const ZPoint &startPlane, double d);
  void moveCutDepth(double d);

  double getCutDepth(const ZPoint &startPlane) const;

  /*
  void fixZ(bool state) {
    m_fixingZ = state;
  }

  bool fixingZ() const {
    return m_fixingZ;
  }
  */

  /*
  const ZViewProj& getViewProj() const {
    return m_viewProj;
  }

  ZArbSliceViewParam getSliceViewParam() const;
  void setArbSliceCenter(const ZIntPoint &pt);
  void setArbSlicePlane(const ZPoint &v1, const ZPoint &v2);
  void setArbSliceView(const ZArbSliceViewParam &param);
  void moveSlice(int step);
  */

//  ZAffinePlane getArbSlicePlane() const;
//  ZAffineRect getSliceRect() const;

  bool onSamePlane(const ZStackViewParam &param) const;

//  std::string toString() const;
  ZJsonObject toJsonObject() const;

  ZSliceViewTransform getSliceViewTransform() const;

private:
//  void init(neutu::ECoordinateSystem coordSys);

  struct ViewportSize {
    void set(int width, int height) {
      m_width = width;
      m_height = height;
    }

    void set(int width, int height, neutu::data3d::ESpace space) {
      set(width, height);
      m_space = space;
    }

    int m_width = 0;
    int m_height = 0;
    neutu::data3d::ESpace m_space = neutu::data3d::ESpace::CANVAS;
  };

private:
  ZSliceViewTransform m_transform;
//  neutu::View::EExploreAction m_action =
//      neutu::View::EExploreAction::EXPLORE_UNKNOWN;

  ViewportSize m_viewportSize;
  ViewportSize m_backupViewportSize;
};

#endif // ZSTACKVIEWPARAM_H
