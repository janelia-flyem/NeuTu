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

  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");

  m_objectWindow = NULL;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc, NeuTube::EAxis axis)
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
  QList<ZDvidSynapseEnsemble*> seList = doc->getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    if (se->getSliceAxis() == frame->getView()->getSliceAxis()) {
      se->attachView(frame->getView());
    }
  }

  QList<ZFlyEmToDoList*> todoList = doc->getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *obj = *iter;
    obj->attachView(frame->getView());
  }

  connect(frame->getPresenter(), SIGNAL(savingStack()),
          frame, SLOT(saveStack()));
  connect(frame->getCompletePresenter(), SIGNAL(highlightModeChanged()),
          frame, SIGNAL(highlightModeChanged()));

  return frame;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, NeuTube::EAxis axis)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc;
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
    m_mergeProject.setDvidTarget(doc->getDvidTarget());
    m_mergeProject.syncWithDvid();
  }
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

