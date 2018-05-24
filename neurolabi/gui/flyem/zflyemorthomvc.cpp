#include "zflyemorthomvc.h"
#include "zflyemorthodoc.h"
#include "zstackpresenter.h"
#include "zflyemsupervisor.h"
#include "zstackview.h"
#include "zwidgetmessage.h"
#include "zflyemproofpresenter.h"
#include "zflyemtodolist.h"

ZFlyEmOrthoMvc::ZFlyEmOrthoMvc(QWidget *parent) :
  ZFlyEmProofMvc(parent)
{
  init();
}

void ZFlyEmOrthoMvc::init()
{
  m_dvidDlg = NULL;
  m_bodyInfoDlg = NULL;
//  m_supervisor = new ZFlyEmSupervisor(this);
  m_splitCommitDlg = NULL;

  m_objectWindow = NULL;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc, neutube::EAxis axis)
{
  ZFlyEmOrthoMvc *frame = new ZFlyEmOrthoMvc(parent);

  BaseConstruct(frame, doc, axis);
  frame->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  frame->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  frame->getView()->setContentsMargins(0, 0, 0, 0);
  frame->getView()->hideThresholdControl();
  frame->getView()->setHoverFocus(true);
  frame->updateDvidTargetFromDoc();
  frame->getPresenter()->useHighContrastProtocal(true);

  ZStackView *view = frame->getView();

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

  connect(frame->getPresenter(), SIGNAL(savingStack()),
          frame, SLOT(saveStack()));
  connect(frame->getCompletePresenter(), SIGNAL(highlightModeChanged()),
          frame, SIGNAL(highlightModeChanged()));

  connect(frame->getPresenter(), SIGNAL(movingCrossHairTo(int,int)),
          frame, SLOT(moveCrossHairTo(int, int)));

  return frame;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, neutube::EAxis axis)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc;
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, neutube::EAxis axis,
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

void ZFlyEmOrthoMvc::moveCrossHairTo(int x, int y)
{
  setCrossHairCenter(x, y);
  getView()->updateImageScreen(ZStackView::UPDATE_QUEUED);
}

ZDvidTarget ZFlyEmOrthoMvc::getDvidTarget() const
{
  return getCompleteDocument()->getDvidTarget();
}

void ZFlyEmOrthoMvc::updateStack(const ZIntPoint &center)
{
  getCompleteDocument()->updateStack(center);
  getView()->updateViewBox();
}

