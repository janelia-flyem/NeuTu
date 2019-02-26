#include "zflyemorthomvc.h"
#include "zflyemorthodoc.h"

#include "logging/zlog.h"
#include "mvc/utilities.h"
#include "mvc/zstackpresenter.h"
#include "mvc/zstackview.h"

#include "zwidgetmessage.h"

#include "zflyemsupervisor.h"
#include "zflyemproofpresenter.h"
#include "zflyemtodolist.h"

ZFlyEmOrthoMvc::ZFlyEmOrthoMvc(QWidget *parent) :
  ZFlyEmProofMvc(parent)
{
  init();
}

void ZFlyEmOrthoMvc::init()
{
//  m_dvidDlg = NULL;
//  m_bodyInfoDlg = NULL;
//  m_supervisor = new ZFlyEmSupervisor(this);
//  m_splitCommitDlg = NULL;

//  m_objectWindow = NULL;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc, neutu::EAxis axis)
{
  ZFlyEmOrthoMvc *mvc = new ZFlyEmOrthoMvc(parent);

  BaseConstruct(mvc, doc, axis);
  mvc->getPresenter()->setObjectStyle(ZStackObject::EDisplayStyle::SOLID);
  mvc->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  mvc->getView()->setContentsMargins(0, 0, 0, 0);
  mvc->getView()->hideThresholdControl();
  mvc->getView()->setHoverFocus(true);
  mvc->getView()->setViewInfoFlag(
        neutu::mvc::ViewInfoFlags(neutu::mvc::ViewInfoFlag::DATA_COORD) |
        neutu::mvc::ViewInfoFlag::IMAGE_VALUE);
  mvc->updateDvidTargetFromDoc();
  mvc->getPresenter()->useHighContrastProtocal(true);

  ZStackView *view = mvc->getView();

  QList<ZDvidSynapseEnsemble*> seList = doc->getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    if (se->getSliceAxis() == view->getSliceAxis()) {
      se->attachView(view);
    }
  }

  QList<ZFlyEmToDoList*> todoList = doc->getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *obj = *iter;
    if (obj->getSliceAxis() == view->getSliceAxis()) {
      obj->attachView(view);
    }
  }

  connect(mvc->getPresenter(), SIGNAL(savingStack()),
          mvc, SLOT(saveStack()));
  connect(mvc->getCompletePresenter(), SIGNAL(highlightModeChanged()),
          mvc, SIGNAL(highlightModeChanged()));

  connect(mvc->getPresenter(), SIGNAL(movingCrossHairTo(int,int)),
          mvc, SLOT(moveCrossHairTo(int, int)));

  return mvc;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, neutu::EAxis axis)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc;
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, neutu::EAxis axis,
    int width, int height, int depth)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc(width, height, depth);
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmOrthoDoc* ZFlyEmOrthoMvc::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmOrthoDoc*>(getDocument().get());
}

void ZFlyEmOrthoMvc::setDvidTarget(const ZDvidTarget &target)
{
  getCompleteDocument()->setDvidTarget(target);

  updateDvidTargetFromDoc();
}

void ZFlyEmOrthoMvc::updateDvidTargetFromDoc()
{
  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    ZDvidReader &reader = doc->getDvidReader();
    if (reader.isReady()) {
      ZJsonObject contrastObj = reader.readContrastProtocal();
      getPresenter()->setHighContrastProtocal(contrastObj);
//      enableSynapseFetcher();
    }

    getView()->updateContrastProtocal();
    getView()->reset(false);
//    if (getSupervisor() != NULL) {
//      getSupervisor()->setDvidTarget(doc->getDvidTarget());
//    }
//    m_mergeProject.setDvidTarget(doc->getDvidTarget());
//    m_mergeProject.syncWithDvid();
  }
}

void ZFlyEmOrthoMvc::setCrossHairCenter(double x, double y)
{
  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    doc->setCrossHairCenter(x, y, getView()->getSliceAxis());
    emit crossHairChanged();
  }
}

void ZFlyEmOrthoMvc::updateStackFromCrossHair()
{

  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    ZPoint pt = doc->getCrossHairCenter();
    pt.setZ(getView()->sliceIndex());
    ZIntPoint dataPos = neutu::mvc::MapWidgetPosToData(getView(), pt).toIntPoint();
    updateStack(dataPos);
  }
}

void ZFlyEmOrthoMvc::moveCrossHairTo(int x, int y)
{
  setCrossHairCenter(x, y);
  getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
}

ZDvidTarget ZFlyEmOrthoMvc::getDvidTarget() const
{
  return getCompleteDocument()->getDvidTarget();
}

void ZFlyEmOrthoMvc::updateStack(const ZIntPoint &center)
{
  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    KINFO << QString("orthogonal view: update data at (%1, %2, %3)").
             arg(center.getX()).arg(center.getY()).arg(center.getZ());

    doc->updateStack(center);
    getView()->updateViewBox();
  }
}

