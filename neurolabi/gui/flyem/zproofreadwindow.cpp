#include "zproofreadwindow.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include "flyemsplitcontrolform.h"
#include "dvid/zdvidtarget.h"
#include "zflyemproofmvc.h"
#include "flyemproofcontrolform.h"

ZProofreadWindow::ZProofreadWindow(QWidget *parent) :
  QMainWindow(parent)
{
  init();
}

void ZProofreadWindow::init()
{
  setAttribute(Qt::WA_DeleteOnClose);

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  ZDvidTarget target;
  target.set("http://emdata1.int.janelia.org", "9db", 8500);

  m_mainMvc = ZFlyEmProofMvc::Make(target);
  m_mainMvc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  layout->addWidget(m_mainMvc);

  m_controlGroup = new QStackedWidget(this);
  layout->addWidget(m_controlGroup);
  m_controlGroup->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  FlyEmProofControlForm *controlForm = new FlyEmProofControlForm;
  m_controlGroup->addWidget(controlForm);

  FlyEmSplitControlForm *splitControlForm = new FlyEmSplitControlForm;
  m_controlGroup->addWidget(splitControlForm);
  splitControlForm->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_mainMvc->connectControlPanel(controlForm);
  m_mainMvc->connectSplitControlPanel(splitControlForm);

  connect(controlForm, SIGNAL(splitTriggered(int64_t)),
          this, SLOT(launchSplit(int64_t)));
  connect(splitControlForm, SIGNAL(exitingSplit()),
          this, SLOT(exitSplit()));

  setCentralWidget(widget);
}

ZProofreadWindow* ZProofreadWindow::Make(QWidget *parent)
{
  return new ZProofreadWindow(parent);
}

void ZProofreadWindow::launchSplit(int64_t bodyId)
{
  m_mainMvc->launchSplit(bodyId);
  m_controlGroup->setCurrentIndex(1);
}

void ZProofreadWindow::exitSplit()
{
  m_mainMvc->exitSplit();
  m_controlGroup->setCurrentIndex(0);
}
