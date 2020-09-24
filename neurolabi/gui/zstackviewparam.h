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
  void setViewport(int width, int height, neutu::data3d::ESpace sizeSpace);

  ZAffineRect getCutRect() const;

  /*!
   * \brief Get a rectangle adjusted for the integer space
   *
   * The result uses the right integer center.
   */
  ZAffineRect getIntCutRect() const;

  /*!
   * \brief Get a rectangle adjusted for the integer space
   *
   * The result will be cropped by \a modelRange. But if \a modeRange is not
   * valid, it is the same is \a getIntCutRect().
   */
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

  /*!
   * \brief Test if the viewport is valid (non-empty)
   */
  bool isValid() const;
  void invalidate();

  double getArea(neutu::data3d::ESpace space) const;

  double getZoomRatio() const;
  int getZoomLevel(int maxLevel) const;
  int getZoomLevel() const;

  /*!
   * \brief Test if the viewport is empty
   */
  bool isViewportEmpty() const;

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

  bool onSamePlane(const ZStackViewParam &param) const;

//  std::string toString() const;
  ZJsonObject toJsonObject() const;

  ZSliceViewTransform getSliceViewTransform() const;

  void setViewId(int id);
  int getViewId() const;

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
  int m_viewId = 0;
//  neutu::View::EExploreAction m_action =
//      neutu::View::EExploreAction::EXPLORE_UNKNOWN;

  ViewportSize m_viewportSize;
//  ViewportSize m_backupViewportSize;
  bool m_isViewportOpen = true;
};

#endif // ZSTACKVIEWPARAM_H
