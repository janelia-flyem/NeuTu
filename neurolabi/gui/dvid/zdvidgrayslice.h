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
      QPainter *painter, const DisplayConfig &config) const override;

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidReader& getDvidReader() const;
  const ZDvidReader& getWorkDvidReader() const;
  const ZDvidTarget& getDvidTarget() const;

  /*!
   * \brief Update the view parameter
   *
   * This function will trigger data update if the current data is out of sync.
   */
  bool update(const ZStackViewParam &viewParam);

  void printInfo() const;

  int getWidth() const;
  int getHeight() const;
  int getZoom() const;

  int getScale() const;

  ZCuboid getBoundBox() const override;

  ZStackViewParam getViewParam() const;
  ZIntCuboid getDataRange() const;


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
  void clear();

  void updateImage(const ZStack *stack, const ZAffinePlane &ap);
  void forceUpdate(const ZStackViewParam &viewParam);
  void updateContrast();

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
  ZImage m_image; //Todo: need to replace ZImage type with a more concise one

  bool m_usingContrastProtocol = false;
  ZContrastProtocol m_contrastProtocol;

  std::unique_ptr<ZDvidDataSliceHelper> m_helper;
};

#endif // ZDVIDGRAYSLICE_H
