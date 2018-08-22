#include "zflyembodysplitter.h"

#include "neutubeconfig.h"
#include "zqslog.h"
#include "zstackdoc.h"
#include "zflyembody3ddoc.h"
#include "zwidgetmessage.h"
#include "zstackwatershedcontainer.h"
#include "dvid/zdvidsparsestack.h"
#include "zstackdocaccessor.h"
#include "zflyemproofdoc.h"

ZFlyEmBodySplitter::ZFlyEmBodySplitter(QObject *parent) : QObject(parent)
{
}

uint64_t ZFlyEmBodySplitter::getBodyId() const
{
  return m_bodyId;
}

void ZFlyEmBodySplitter::setBodyId(uint64_t bodyId)
{
  if (m_bodyId != bodyId) {
    m_bodyId = bodyId;
    m_state = STATE_NO_SPLIT;
  }
}

void ZFlyEmBodySplitter::setBody(uint64_t bodyId, flyem::EBodyLabelType type)
{
  if (m_bodyId != bodyId || m_labelType != type) {
    m_bodyId = bodyId;
    m_labelType = type;
    m_state = STATE_NO_SPLIT;
  }
}

flyem::EBodyLabelType ZFlyEmBodySplitter::getLabelType() const
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
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::RANGE_SEED);
}

void ZFlyEmBodySplitter::runFullSplit()
{
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::RANGE_FULL);
}

void ZFlyEmBodySplitter::runLocalSplit()
{
  runSplit(getParentDoc<ZFlyEmBody3dDoc>(), flyem::RANGE_LOCAL);
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
  resetSplitState();

  if (doc != NULL) {
    if (doc->isSplitActivated()) {
      notifyWindowMessageUpdated("Running split ...");
      doc->loadDvidSparseStackForSplit();

      QList<ZStackObject*> seedList =
          doc->getObjectList(ZStackObjectRole::ROLE_SEED);
      if (seedList.size() > 1) {
        ZStackWatershedContainer container(NULL, NULL);
        foreach (ZStackObject *seed, seedList) {
          container.addSeed(seed);
        }
        container.setCcaPost(true);

        ZIntCuboid dataRange;
        switch (rangeOption) {
        case flyem::RANGE_FULL:
          container.setRangeHint(ZStackWatershedContainer::RANGE_FULL);
          break;
        case flyem::RANGE_SEED:
          container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_ROI);
          break;
        case flyem::RANGE_LOCAL:
          container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_BOUND);
          dataRange = container.getRange();
          break;
        }

        ZDvidSparseStack *sparseStack =
            doc->getDataDocument()->getDvidSparseStack(
              dataRange, flyem::BODY_SPLIT_ONLINE);
        if (sparseStack != NULL) {
          container.setData(
                NULL, sparseStack->getSparseStack(container.getRange()));
          if (rangeOption == flyem::RANGE_LOCAL) {
            std::vector<ZStackWatershedContainer*> containerList =
                container.makeLocalSeedContainer(256);

            ZOUT(LINFO(), 5) << containerList.size() << "containers";
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
}

ZFlyEmBodySplitter::EState ZFlyEmBodySplitter::getState() const
{
  return m_state;
}

void ZFlyEmBodySplitter::updateSplitState(flyem::EBodySplitRange rangeOption)
{
  switch (rangeOption) {
  case flyem::RANGE_FULL:
    m_state = STATE_FULL_SPLIT;
    break;
  case flyem::RANGE_SEED:
    m_state = STATE_SPLIT;
    break;
  case flyem::RANGE_LOCAL:
    m_state = STATE_LOCAL_SPLIT;
    break;
  }
}

void ZFlyEmBodySplitter::resetSplitState()
{
  m_state = STATE_NO_SPLIT;
}

void ZFlyEmBodySplitter::notifyWindowMessageUpdated(const QString &message)
{
  emit messageGenerated(
        ZWidgetMessage(
          message, neutube::MSG_INFORMATION,
          ZWidgetMessage::TARGET_CUSTOM_AREA));
}
