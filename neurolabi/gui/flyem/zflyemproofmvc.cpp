#include "zflyemproofmvc.h"

#include "flyem/zflyemproofdoc.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackpresenter.h"
#include "zdviddialog.h"
#include "dvid/zdvidreader.h"
#include "zstackobjectsourcefactory.h"
#include "dvid/zdvidsparsestack.h"

ZFlyEmProofMvc::ZFlyEmProofMvc(QWidget *parent) :
  ZStackMvc(parent), m_splitOn(false)
{
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc)
{
  ZFlyEmProofMvc *frame = new ZFlyEmProofMvc(parent);

  BaseConstruct(frame, doc);

  return frame;
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(const ZDvidTarget &target)
{
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc(NULL, NULL);
  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmProofMvc *mvc =
      ZFlyEmProofMvc::Make(NULL, ZSharedPointer<ZFlyEmProofDoc>(doc));
  mvc->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmProofDoc* ZFlyEmProofMvc::getCompleteDocument() const
{
  return dynamic_cast<ZFlyEmProofDoc*>(getDocument().get());
}

void ZFlyEmProofMvc::mergeSelected()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->mergeSelected();
  }
}

void ZFlyEmProofMvc::undo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->undo();
  }
}

void ZFlyEmProofMvc::redo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->redo();
  }
}

void ZFlyEmProofMvc::setSegmentationVisible(bool visible)
{
  m_showSegmentation = visible;
  if (getCompleteDocument() != NULL) {
    QList<ZDvidLabelSlice*> sliceList =
        getCompleteDocument()->getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *slice, sliceList) {
      slice->setVisible(visible);
      slice->update();
      /*
      if (visible) {
        slice->update();
      }
      */
    }
  }
  getView()->redrawObject();
}

void ZFlyEmProofMvc::clear()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->clearData();
    getPresenter()->clearData();
//    getView()->imageWidget();
  }
}

void ZFlyEmProofMvc::setDvidTarget(const ZDvidTarget &target)
{
  if (getCompleteDocument() != NULL) {
    clear();
//    getCompleteDocument()->clearData();
    getCompleteDocument()->setDvidTarget(target);
    getCompleteDocument()->updateTileData();
    QList<ZDvidTileEnsemble*> teList =
        getCompleteDocument()->getDvidTileEnsembleList();
    foreach (ZDvidTileEnsemble *te, teList) {
      te->attachView(getView());
    }
    getView()->reset(false);

    m_splitProject.setDvidTarget(target);
  }
}

void ZFlyEmProofMvc::setDvidTarget()
{
  m_dvidDlg = new ZDvidDialog(this);
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    setDvidTarget(target);

  }
}

const ZDvidTarget& ZFlyEmProofMvc::getDvidTarget() const
{
  return m_dvidDlg->getDvidTarget();
}

void ZFlyEmProofMvc::customInit()
{
  connect(getPresenter(), SIGNAL(bodySplitTriggered()),
          this, SLOT(notifySplitTriggered()));
  m_splitProject.setDocument(getDocument());
  connect(&m_splitProject, SIGNAL(locating2DViewTriggered(const ZStackViewParam&)),
          this->getView(), SLOT(setView(const ZStackViewParam&)));

  m_mergeProject.setDocument(getDocument());

  connect(getDocument().get(), SIGNAL(messageGenerated(const QString&)),
          this, SIGNAL(messageGenerated(const QString&)));

}

void ZFlyEmProofMvc::notifySplitTriggered()
{
  ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

  if (labelSlice->isVisible()) {
    const std::set<int64_t> &selected = labelSlice->getSelected();

    if (!selected.empty()) {
      int64_t bodyId = *(selected.begin());

      emit launchingSplit(bodyId);
    }
  }
}

bool ZFlyEmProofMvc::launchSplit(int64_t bodyId)
{
  if (bodyId >= 0) {
    if (!getCompleteDocument()->isSplittable(bodyId)) {
      return false;
    }

    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      if (body != NULL) {
        if ((int64_t) body->getLabel() != bodyId) {
          body = NULL;
        }
      }

      ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
      if (body == NULL) {
        body = reader.readDvidSparseStack(bodyId);
        body->setZOrder(0);
//        body = new ZDvidSparseStack;

//        reader.readBody(bodyId, body);
        body->setSource(ZStackObjectSourceFactory::MakeSplitObjectSource());
        body->setMaskColor(labelSlice->getColor(bodyId));
        getDocument()->addObject(body, true);
      }

      labelSlice->setVisible(false);
      labelSlice->setHittable(false);
      body->setVisible(true);
      getView()->redrawObject();

      m_splitOn = true;
      m_splitProject.setBodyId(bodyId);

      return true;
    }
  }

  return false;
}

void ZFlyEmProofMvc::exitSplit()
{
  if (m_splitOn) {
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    labelSlice->setVisible(true);
    labelSlice->setHittable(true);

    getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);

    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    if (body != NULL) {
      body->setVisible(false);
    }
    getView()->redrawObject();
    m_splitProject.clear();

    m_splitOn = false;
  }
}

void ZFlyEmProofMvc::processMessageSlot(const QString &message)
{
  ZJsonObject obj;
  obj.decodeString(message.toStdString().c_str());

  if (obj.hasKey("type")) {
    std::string type = ZJsonParser::stringValue(obj["type"]);
    if (type == "locate_view") {
//      viewRoi(int x, int y, int z, int radius);
    }
  }
}

void ZFlyEmProofMvc::showBodyQuickView()
{
  m_splitProject.quickView();
}

//void ZFlyEmProofMvc::toggleEdgeMode(bool edgeOn)


