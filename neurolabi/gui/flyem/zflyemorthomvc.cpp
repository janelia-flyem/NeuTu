#include "zflyemorthomvc.h"

#include "logging/zlog.h"
#include "data3d/displayconfig.h"

#include "mvc/utilities.h"
#include "mvc/zstackpresenter.h"
#include "mvc/zstackview.h"

#include "zwidgetmessage.h"

#include "zflyemorthodoc.h"
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

  ZStackView *view = mvc->getMainView();
  view->layout()->setContentsMargins(0, 0, 0, 0);
  view->setContentsMargins(0, 0, 0, 0);
  view->hideThresholdControl();
  view->setHoverFocus(true);
  view->setViewInfoFlag(
        neutu::mvc::ViewInfoFlags(neutu::mvc::ViewInfoFlag::DATA_COORD) |
        neutu::mvc::ViewInfoFlag::IMAGE_VALUE);
  mvc->updateDvidTargetFromDoc();
  mvc->getPresenter()->useHighContrastProtocal(true);


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

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(const ZDvidEnv &env, neutu::EAxis axis)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc;
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvid(env);

  return mvc;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidEnv &env, neutu::EAxis axis,
    int width, int height, int depth)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc(width, height, depth);
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvid(env);

  return mvc;
}

ZFlyEmOrthoDoc* ZFlyEmOrthoMvc::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmOrthoDoc*>(getDocument().get());
}

/*
void ZFlyEmOrthoMvc::setDvidTarget(const ZDvidTarget &target)
{
  getCompleteDocument()->setDvidTarget(target);

  updateDvidTargetFromDoc();
}*/

void ZFlyEmOrthoMvc::setDvid(const ZDvidEnv &env)
{
  if (getCompleteDocument()->setDvid(env)) {
    updateDvidTargetFromDoc();
  }
}

void ZFlyEmOrthoMvc::updateDvidTargetFromDoc()
{
  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    ZDvidReader &reader = doc->getDvidReader();
    if (reader.isReady()) {
//      ZJsonObject contrastObj = reader.readContrastProtocal();
      getPresenter()->setHighContrastProtocal(
            doc->getContrastProtocol().toJsonObject());
//      enableSynapseFetcher();
    }

    getMainView()->updateContrastProtocal();
//    getView()->reset(false);
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
    doc->setCrossHairCenter(x, y, getMainView()->getSliceAxis());
    emit crossHairChanged();
  }
}

void ZFlyEmOrthoMvc::updateStackFromCrossHair()
{

  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    ZPoint dataPos = getMainView()->getAnchorPoint(neutu::data3d::ESpace::MODEL);
//    ZPoint pt = doc->getCrossHairCenter();
//    pt.setZ(getView()->sliceIndex());
//    ZIntPoint dataPos = neutu::mvc::MapWidgetPosToData(getView(), pt).toIntPoint();
    updateStack(dataPos.roundToIntPoint());
  }
}

void ZFlyEmOrthoMvc::moveCrossHairTo(int x, int y)
{
  setCrossHairCenter(x, y);
  getMainView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
}

/*
ZDvidTarget ZFlyEmOrthoMvc::getDvidTarget() const
{
  return getCompleteDocument()->getDvidTarget();
}
*/

void ZFlyEmOrthoMvc::updateStack(const ZIntPoint &center)
{
  ZFlyEmOrthoDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    KINFO(neutu::TOPIC_NULL) << QString("orthogonal view: update data at (%1, %2, %3)").
             arg(center.getX()).arg(center.getY()).arg(center.getZ());

    doc->updateStack(center);
    getMainView()->updateViewBox();
  }
}

void ZFlyEmOrthoMvc::processViewChangeCustom(const ZStackViewParam &viewParam)
{
  if (/*getView()->getSliceAxis() == neutu::EAxis::Z &&*/
      viewParam == getMainView()->getViewParameter() && m_autoReload) {
    ZFlyEmOrthoDoc *doc = getCompleteDocument();
    if (doc != NULL) {
//      ZPoint pt = doc->getCrossHairCenter();
//      pt.setZ(getView()->sliceIndex());
//      ZIntPoint dataPos = neutu::mvc::MapWidgetPosToData(getView(), pt).toIntPoint();
      ZIntPoint dataPos = getMainView()->getAnchorPoint(
            neutu::data3d::ESpace::MODEL).roundToIntPoint();
      if (!doc->getDataRange().contains(dataPos)) {
        doc->updateStack(dataPos);
//        updateStack(dataPos);
      }
    }
  }
}

