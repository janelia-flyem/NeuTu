#ifndef ZDVIDGRAYSLICE_H
#define ZDVIDGRAYSLICE_H

#include <memory>

#include "zstackobject.h"
#include "zimage.h"
//#include "zdvidreader.h"
#include "zstackviewparam.h"
#include "zarbsliceviewparam.h"
#include "zpixmap.h"
#include "zcontrastprotocol.h"
#include "zuncopyable.h"
#include "vis2d/zslicecanvas.h"

//#include "zdvidtarget.h"

class ZRect2d;
class ZDvidReader;
class ZStack;
class ZDvidDataSliceHelper;
class ZDvidTarget;
class ZTask;
class ZStackDoc;
//class ZStackViewParam;

class ZDvidGraySlice : public ZStackObject, ZUncopyable
{
public:
  ZDvidGraySlice();
  ~ZDvidGraySlice();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_GRAY_SLICE;
  }

  bool display(
      QPainter *painter, const DisplayConfig &config) const;
  /*
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
               */
  void clear();

//  void update(int z);
  bool update(const ZStackViewParam &viewParam);
  /*!
   * \brief Update an arbitrary cutting plane
   *
   * It only takes effect only when the axis is neutube::A_AXIS.
   */
//  bool update(const ZArbSliceViewParam &viewParam);

//  void loadDvidSlice(const QByteArray &buffer, int z);

//  virtual const std::string& className() const;

  void printInfo() const;

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidReader& getDvidReader() const;
  const ZDvidReader& getWorkDvidReader() const;

  const ZDvidTarget& getDvidTarget() const;
//  int getX() const;
//  int getY() const;
  int getZ() const;
//  void setZ(int z);
  int getWidth() const;
  int getHeight() const;
  int getZoom() const;

  int getScale() const;

  /*
  inline const ZDvidTarget& getDvidTarget() const {
    return m_reader.getDvidTarget();
  }

  inline int getX() const {
    return m_currentViewParam.getViewPort().left();
  }
  inline int getY() const {
    return m_currentViewParam.getViewPort().top();
  }
  inline int getZ() const {
    return m_currentViewParam.getZ();
  }
  inline void setZ(int z) {
    m_currentViewParam.setZ(z);
  }
  */

//  int getWidth() const { return m_currentViewParam.getViewPort().width(); }
//  int getHeight() const { return m_currentViewParam.getViewPort().height(); }

  ZCuboid getBoundBox() const override;
//  ZRect2d getBoundBox() const;
//  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

//  void setBoundBox(const ZRect2d &rect);

//  QRect getViewPort() const;
  ZStackViewParam getViewParam() const;


  void setZoom(int zoom);
  void setContrastProtocol(const ZContrastProtocol &cp);
  void updateContrast(bool highContrast);
  void updateContrast(const ZJsonObject &obj);

  /*!
   * \brief Check if the slice has any low-resolution region.
   */
  bool hasLowresRegion() const;

  void setCenterCut(int width, int height);

  /*
  const ZPixmap& getPixmap() const {
    return m_pixmap;
  }
  */
  const QImage getImage() const {
    return m_imageCanvas.toImage();
  }

//  void setArbitraryAxis(const ZPoint &v1, const ZPoint &v2);

  bool consume(ZStack *stack, const ZStackViewParam &viewParam,
               int zoom, int centerCutX, int centerCutY, bool usingCenterCut);
  bool containedIn(const ZStackViewParam &viewParam, int zoom,
                   int centerCutX, int centerCutY, bool centerCut) const;
  ZTask* makeFutureTask(ZStackDoc *doc);

public: //for testing
  void saveImage(const std::string &path);
  void savePixmap(const std::string &path);
//  void test();

private:
//  void updateImage();
  void updateImage(const ZStack *stack, const ZAffineRect &rect);
  void forceUpdate(const ZStackViewParam &viewParam);
//  void forceUpdate(const QRect &viewPort, int z);
//  void forceUpdate(const ZArbSliceViewParam &sliceViewParam);

//  void updatePixmap();
  void updateContrast();
//  void invalidateCanvas();
//  void validatePixmap(bool v);
//  void validatePixmap();
//  bool isCanvasValid() const;

//  bool validateSize(int *width, int *height);
  template<typename T>
  int updateParam(T *param);

  /*!
   * \brief Check if the regions of the image and the slice are consistent.
   */
//  bool isRegionConsistent() const;

  const ZDvidDataSliceHelper* getHelper() const {
    return m_helper.get();
  }
  ZDvidDataSliceHelper* getHelper() {
    return m_helper.get();
  }

private:
  ZSliceCanvas m_imageCanvas;
  ZImage m_image;
//  ZImage m_image;
//  ZPixmap m_pixmap;
//  bool m_isCanvasValid = false;

  bool m_usingContrastProtocol = false;
  ZContrastProtocol m_contrastProtocol;

//  QMutex m_pixmapMutex;
//  ZStackViewParam m_currentViewParam;

//  ZArbSliceViewParam m_sliceViewParam; //Only useful for A_AXIS

//  int m_zoom;

//  int m_maxWidth;
//  int m_maxHeight;

  std::unique_ptr<ZDvidDataSliceHelper> m_helper;
//  ZPoint m_v1;
//  ZPoint m_v2;

//  ZDvidReader m_reader;
};

#endif // ZDVIDGRAYSLICE_H
