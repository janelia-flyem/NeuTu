#include "zflyemarbdoc.h"
#include "zwidgetmessage.h"
#include "zstackfactory.h"
#include "dvid/zdvidgrayslice.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidenv.h"

#include "zstackobjectsourcefactory.h"

ZFlyEmArbDoc::ZFlyEmArbDoc(QObject *parent) : ZFlyEmProofDoc(parent)
{
  setTag(neutu::Document::ETag::FLYEM_ARBSLICE);
}

bool ZFlyEmArbDoc::setDvid(const ZDvidEnv &env)
{
  m_originalEnv = env;

  if (m_dvidWriter.open(env.getFullMainTarget())) {
    m_dvidEnv = env;
    prepareGrayscaleReader();

    m_activeBodyColorMap.reset();
    m_mergeProject->setDvidTarget(getDvidTarget());

    readInfo();

    prepareDvidData(env);

#ifdef _DEBUG_2
    ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
    bookmark->setCenter(1352, 1319, 890);
    bookmark->setZScale(4);
    addObject(bookmark);
#endif

//    downloadBookmark();

    return true;
  }else {
    m_dvidWriter.clear();
    emit messageGenerated(
          ZWidgetMessage("Failed to open the node.", neutu::EMessageType::ERROR));
  }

  return false;
}

#if 0
void ZFlyEmArbDoc::setDvidTarget(const ZDvidTarget &target)
{
  if (m_dvidWriter.open(target)) {
//    m_dvidWriter.open(target);
    m_grayscaleReader.openRaw(getDvidTarget().getGrayScaleTarget());
    m_activeBodyColorMap.reset();
    m_mergeProject->setDvidTarget(getDvidTarget());

    readInfo();

    ZDvidEnv env;
    env.set(target);
    prepareDvidData(env);
  } else {
    m_dvidWriter.clear();
//    m_dvidTarget.clear();
    emit messageGenerated(
          ZWidgetMessage("Failed to open the node.", neutu::EMessageType::ERROR));
  }
}
#endif

void ZFlyEmArbDoc::prepareDvidData(const ZDvidEnv &env)
{
  if (getDvidReader().isReady()) {
    ZDvidInfo dvidInfo = getDvidInfo();

    ZIntCuboid boundBox;
    if (dvidInfo.isValid()) {
      boundBox = ZIntCuboid(dvidInfo.getStartCoordinates(),
                       dvidInfo.getEndCoordinates());
    } else {
      boundBox = ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(512, 512, 512));
    }

//    int radius = int(std::ceil(boundBox.getDiagonalLength() * 0.5));
//    ZIntPoint center = boundBox.getCenter();
//    ZIntCuboid newBox(center - radius, center + radius);
//    newBox.setFirstZ(0);
//    newBox.setDepth(1);

    ZStack *stack = ZStackFactory::MakeVirtualStack(boundBox);
    loadStack(stack);

    if (getDvidTarget().hasGrayScaleData()) {
      initGrayscaleSlice(env, neutu::EAxis::ARB);
      /*
      ZDvidGraySlice *slice = new ZDvidGraySlice;
      slice->setSliceAxis(neutu::EAxis::ARB);
      slice->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      slice->setSource(
            ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::ARB));
      slice->setDvidTarget(m_grayscaleReader.getDvidTarget());
      prepareGraySlice(slice);
      addObject(slice, true);
      */
    }

    if (getDvidTarget().hasSegmentation()) {
      addDvidLabelSlice(neutu::EAxis::ARB, false);
#if 0
      ZDvidLabelSlice *slice = new ZDvidLabelSlice;
      slice->setSliceAxis(neutu::EAxis::ARB);
      slice->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      slice->setSource(
            ZStackObjectSourceFactory::MakeDvidLabelSliceSource(
              neutu::EAxis::ARB, false));
      slice->setDvidTarget(getDvidTarget());
//      prepareGraySlice(slice);
      addObject(slice, true);
#endif
    }
  }
}
