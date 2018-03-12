#include "zflyemarbdoc.h"
#include "zwidgetmessage.h"
#include "zstackfactory.h"
#include "dvid/zdvidgrayslice.h"
#include "zstackobjectsourcefactory.h"

ZFlyEmArbDoc::ZFlyEmArbDoc(QObject *parent) : ZFlyEmProofDoc(parent)
{
}

void ZFlyEmArbDoc::setDvidTarget(const ZDvidTarget &target)
{
  if (m_dvidReader.open(target)) {
    m_dvidWriter.open(target);
    m_grayscaleReader.openRaw(m_dvidReader.getDvidTarget().getGrayScaleTarget());
    m_activeBodyColorMap.reset();
    m_mergeProject->setDvidTarget(m_dvidReader.getDvidTarget());

    readInfo();

    prepareDvidData();
  } else {
    m_dvidReader.clear();
//    m_dvidTarget.clear();
    emit messageGenerated(
          ZWidgetMessage("Failed to open the node.", neutube::MSG_ERROR));
  }
}

void ZFlyEmArbDoc::prepareDvidData()
{
  if (m_dvidReader.isReady()) {
    ZDvidInfo dvidInfo = getDvidInfo();

    ZIntCuboid boundBox;
    if (dvidInfo.isValid()) {
      boundBox = ZIntCuboid(dvidInfo.getStartCoordinates(),
                       dvidInfo.getEndCoordinates());
    } else {
      boundBox = ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(512, 512, 512));
    }

    int radius = int(std::ceil(boundBox.getDiagonalLength() * 0.5));
    ZIntPoint center = boundBox.getCenter();
    ZIntCuboid newBox(center - radius, center + radius);
//    newBox.setFirstZ(0);
//    newBox.setDepth(1);

    ZStack *stack = ZStackFactory::MakeVirtualStack(newBox);
    loadStack(stack);

    if (getDvidTarget().hasGrayScaleData()) {
      ZDvidGraySlice *slice = new ZDvidGraySlice;
      slice->setSliceAxis(neutube::A_AXIS);
      slice->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      slice->setSource(
            ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::A_AXIS));
      slice->setDvidTarget(m_grayscaleReader.getDvidTarget());
      prepareGraySlice(slice);
      addObject(slice, true);
    }
  }
}
