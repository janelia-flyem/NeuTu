#include "zflyembodysplitter.h"

#include <QElapsedTimer>

#include "common/neutudefs.h"
#include "logging/zlog.h"
#include "logging/utilities.h"

#include "neutubeconfig.h"
#include "logging/zqslog.h"
#include "mvc/zstackdoc.h"
#include "zflyembody3ddoc.h"
#include "zwidgetmessage.h"
#include "zstackwatershedcontainer.h"
#include "dvid/zdvidsparsestack.h"
#include "zstackdocaccessor.h"
#include "zflyemproofdoc.h"
#include "zsparsestack.h"

ZFlyEmBodySplitter::ZFlyEmBodySplitter(QObject *parent) : QObject(parent)
{
}

ZFlyEmBodySplitter::~ZFlyEmBodySplitter()
{
  invalidateCache();
}

void ZFlyEmBodySplitter::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
}

uint64_t ZFlyEmBodySplitter::getBodyId() const
{
  return m_bodyId;
}

void ZFlyEmBodySplitter::setBodyId(uint64_t bodyId)
{
  if (m_bodyId != bodyId) {
    m_bodyId = bodyId;
    m_state = EState::STATE_NO_SPLIT;
  }
}

void ZFlyEmBodySplitter::setBody(uint64_t bodyId, neutu::EBodyLabelType type, bool fromTar)
{
  if (m_bodyId != bodyId || m_labelType != type) {
    m_bodyId = bodyId;
    m_labelType = type;
    m_state = EState::STATE_NO_SPLIT;
    m_fromTar = fromTar;
  }
}

neutu::EBodyLabelType ZFlyEmBodySplitter::getLabelType() const
{
  return m_labelType;
}

template<typename T>
T* ZFlyEmBodySplitter::getParentDoc() const
{
  return qobject_cast<T*>(parent());
}

void ZFlyEmBodySplitter::runSplit()
{
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::EBodySplitRange::SEED);
}

void ZFlyEmBodySplitter::runFullSplit()
{
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::EBodySplitRange::FULL);
}

void ZFlyEmBodySplitter::runLocalSplit()
{
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::EBodySplitRange::LOCAL);
}
/*
void ZFlyEmBodySplitter::runSplit(ZFlyEmBody3dDoc *doc)
{
  runSplit(doc, ZStackWatershedContainer::RANGE_SEED_ROI);
}
*/

void ZFlyEmBodySplitter::runSplit(
    ZFlyEmBody3dDoc *doc, flyem::EBodySplitRange rangeOption)
{
  QElapsedTimer timer;
  timer.start();

  resetSplitState();

  if (doc != NULL) {
    if (doc->isSplitActivated()) {
      notifyWindowMessageUpdated("Running split ...");
//      doc->loadDvidSparseStackForSplit();



      QList<ZStackObject*> seedList =
          doc->getObjectList(ZStackObjectRole::ROLE_SEED);
      if (seedList.size() > 1) {
        ZStackWatershedContainer container(NULL, NULL);
        container.setProfileLogger(&neutu::LogProfileInfo);
        foreach (ZStackObject *seed, seedList) {
          container.addSeed(seed);
        }
        container.setCcaPost(true);

        ZIntCuboid dataRange;
        switch (rangeOption) {
        case flyem::EBodySplitRange::FULL:
          container.setRangeHint(ZStackWatershedContainer::RANGE_FULL);
          break;
        case flyem::EBodySplitRange::SEED:
          container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_ROI);
          break;
        case flyem::EBodySplitRange::LOCAL:
          container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_BOUND);
          dataRange = container.getRange();
          break;
        }

        ZSparseStack *sparseStack = getBodyForSplit();
//        ZDvidSparseStack *sparseStack =
//            doc->getDataDocument()->getDvidSparseStack(
//              dataRange, neutu::EBodySplitMode::BODY_SPLIT_ONLINE);
        if (sparseStack != NULL) {
          container.setData(NULL, sparseStack);
//                NULL, sparseStack->getSparseStack(container.getRange()));
          if (rangeOption == flyem::EBodySplitRange::LOCAL) {
            std::vector<ZStackWatershedContainer*> containerList =
                container.makeLocalSeedContainer(256);

            KINFO << QString("%1 watershed containers").arg(containerList.size());
            for (ZStackWatershedContainer *subcontainer : containerList) {
              subcontainer->run();
              ZStackDocAccessor::ParseWatershedContainer(doc, subcontainer);
              delete subcontainer;
            }
          } else {
            container.run();
            ZStackDocAccessor::ParseWatershedContainer(doc, &container);
          }

          updateSplitState(rangeOption);

          notifyWindowMessageUpdated("Split finished.");
        } else {
          notifyWindowMessageUpdated("No stack data found.");
        }
      } else {
        //    std::cout << "Less than 2 seeds found. Abort." << std::endl;
        notifyWindowMessageUpdated("Less than 2 seeds found. Split canceled.");
      }
    } else {
      notifyWindowMessageUpdated("Failed to load body data. Split aborted.");
    }
    doc->releaseBody(getBodyId(), getLabelType());
  }

  LKINFO << QString("Splitting time for %1 (%2) with range %3: %4ms")
            .arg(getBodyId()).arg(neutu::ToString(getLabelType()).c_str())
            .arg(neutu::EnumValue(rangeOption)).arg(timer.elapsed());
//  LINFO() << "Splitting time:" << timer.elapsed() << "ms";
}

ZFlyEmBodySplitter::EState ZFlyEmBodySplitter::getState() const
{
  return m_state;
}

bool ZFlyEmBodySplitter::fromTar() const
{
  return m_fromTar;
}

void ZFlyEmBodySplitter::setFromTar(bool status)
{
  m_fromTar = status;
}

void ZFlyEmBodySplitter::updateSplitState(flyem::EBodySplitRange rangeOption)
{
  switch (rangeOption) {
  case flyem::EBodySplitRange::FULL:
    m_state = EState::STATE_FULL_SPLIT;
    break;
  case flyem::EBodySplitRange::SEED:
    m_state = EState::STATE_SPLIT;
    break;
  case flyem::EBodySplitRange::LOCAL:
    m_state = EState::STATE_LOCAL_SPLIT;
    break;
  }
}

void ZFlyEmBodySplitter::resetSplitState()
{
  m_state = EState::STATE_NO_SPLIT;
}

void ZFlyEmBodySplitter::invalidateCache()
{
  delete m_cachedObject;
  m_cachedObject = nullptr;
  m_cachedBodyId = 0;
  m_cachedLabelType = neutu::EBodyLabelType::BODY;
}

void ZFlyEmBodySplitter::cacheBody(ZSparseStack *body)
{
  invalidateCache();
  m_cachedObject = body;
  m_cachedBodyId = m_bodyId;
  m_cachedLabelType = m_labelType;
}

ZSparseStack* ZFlyEmBodySplitter::getBodyForSplit()
{
  ZSparseStack *spStack = nullptr;
  if (m_bodyId == m_cachedBodyId && m_labelType == m_cachedLabelType) {
    spStack = m_cachedObject;
  }

  if (spStack == nullptr) {
    QElapsedTimer timer;
    timer.start();
    spStack = m_reader.readSparseStackOnDemand(m_bodyId, m_labelType, NULL);
    neutu::LogProfileInfo(
          timer.elapsed(),
          "Load body for split: " + std::to_string(m_bodyId) + " " +
          neutu::ToString(m_labelType));

    cacheBody(spStack);
  }

  return spStack;
}

void ZFlyEmBodySplitter::updateCachedMask(ZObject3dScan *obj)
{
  if (m_cachedObject != NULL) {
    m_cachedObject->setObjectMask(obj);
  }
}

void ZFlyEmBodySplitter::notifyWindowMessageUpdated(const QString &message)
{
  emit messageGenerated(
        ZWidgetMessage(
          message, neutu::EMessageType::INFORMATION,
          ZWidgetMessage::TARGET_CUSTOM_AREA));
}
