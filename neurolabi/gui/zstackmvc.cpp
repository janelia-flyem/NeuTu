#include "zstackmvc.h"
#include "zstackdoc.h"
#include "zstackview.h"
#include "zstackpresenter.h"

ZStackMvc::ZStackMvc(QWidget *parent) :
  QWidget(parent)
{
  m_view = NULL;
  m_presenter = NULL;
  m_layout = new QHBoxLayout(this);
}

ZStackMvc* ZStackMvc::Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc);

  return frame;
}

void ZStackMvc::construct(ztr1::shared_ptr<ZStackDoc> doc)
{
  dropDocument(ZSharedPointer<ZStackDoc>(doc));
  createView();
  createPresenter();
  updateDocument();

  //m_view->prepareDocument();
  m_presenter->prepareView();
}

void ZStackMvc::BaseConstruct(ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc)
{
  frame->construct(doc);
}

void ZStackMvc::createView()
{
  if (m_doc.get() != NULL) {
    //ZIntPoint size = m_doc->getStackSize();
    m_view = new ZStackView(dynamic_cast<QWidget*>(this));
    m_layout->addWidget(m_view);
  }
}

void ZStackMvc::createPresenter()
{
  if (m_doc.get() != NULL) {
    m_presenter = new ZStackPresenter(this);
  }
}

void ZStackMvc::attachDocument(ZStackDoc *doc)
{
  attachDocument(ZSharedPointer<ZStackDoc>(doc));
}

void ZStackMvc::attachDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_doc = doc;
}

#define UPDATE_DOC_SIGNAL_SLOT(connect) \
  connect(m_doc.get(), SIGNAL(stackLoaded()), this, SIGNAL(stackLoaded()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateChannelControl()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateThresholdSlider()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateSlider()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_presenter, SLOT(updateStackBc()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateView()));\
  connect(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(cleanChanged(bool)),\
          this, SLOT(changeWindowTitle(bool)));\
  connect(m_doc.get(), SIGNAL(holdSegChanged()), m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(chainSelectionChanged(QList<ZLocsegChain*>,\
          QList<ZLocsegChain*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged()),\
          this, SLOT(updateSwcExtensionHint()));\
  connect(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(\
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(objectSelectionChanged(\
                                QList<ZStackObject*>,QList<ZStackObject*>)),\
          m_view, SLOT(paintObject(QList<ZStackObject*>,QList<ZStackObject*>)));\
  connect(m_doc.get(), SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(punctumVisibleStateChanged()),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),\
          this, SLOT(notifyUser(QString)));\
  connect(m_doc.get(), SIGNAL(stackTargetModified()), m_view, SLOT(paintStack()));\
  connect(m_doc.get(), SIGNAL(thresholdChanged(int)), m_view, SLOT(setThreshold(int)));\
  connect(m_view, SIGNAL(viewChanged(ZStackViewParam)), \
          this, SLOT(processViewChange(ZStackViewParam)));

#define UPDATE_SIGNAL_SLOT(connect) \
  UPDATE_DOC_SIGNAL_SLOT(connect) \
  connect(m_view, SIGNAL(currentSliceChanged(int)),\
          m_presenter, SLOT(processSliceChangeEvent(int)));

  //connect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));

void ZStackMvc::connectSignalSlot()
{
  UPDATE_SIGNAL_SLOT(connect);
}

void ZStackMvc::disconnectAll()
{
  UPDATE_SIGNAL_SLOT(disconnect);
}

void ZStackMvc::dropDocument(ztr1::shared_ptr<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    if (m_doc.get() != NULL) {
      UPDATE_DOC_SIGNAL_SLOT(disconnect);
    }
    m_doc = doc;
    //m_doc->setParentFrame(this);
  }
}

void ZStackMvc::updateDocument()
{
  UPDATE_DOC_SIGNAL_SLOT(connect);

  /*
  m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                              xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                       zResolution(), unit(),
                                       reconstructDistThre(),
                                       reconstructSpTest(),
                                       crossoverTest());
                                       */

  if (m_doc->hasStackData()) {
    if (m_presenter != NULL) {
      m_presenter->optimizeStackBc();
    }

    if (m_view != NULL) {
      m_view->reset();
    }
  }

  //m_progressReporter.setProgressBar(m_view->progressBar());
  //m_doc->setProgressReporter(&m_progressReporter);
}

void ZStackMvc::keyPressEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    m_presenter->processKeyPressEvent(event);
  }
}

void ZStackMvc::processViewChange(const ZStackViewParam &viewParam)
{
  QList<const ZDocPlayer*> playerList =
      m_doc->getPlayerList(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  if (!playerList.isEmpty()) {
    bool updated = false;
    foreach (const ZDocPlayer *player, playerList) {
      if (player->getData()->isVisible()) {
        updated = true;
      }
      player->updateData(viewParam);
    }
    if (updated) {
      m_view->paintObject();
    }
  }
}
