#include "zflyemorthowindow.h"

#include <QStatusBar>

#include "logging/zlog.h"

#include "flyem/zflyemorthowidget.h"
#include "flyem/zflyemorthodoc.h"

ZFlyEmOrthoWindow::ZFlyEmOrthoWindow(const ZDvidEnv &env, QWidget *parent) :
  QMainWindow(parent)
{
  m_orthoWidget = new ZFlyEmOrthoWidget(env, this);

  initWidget();
}

ZFlyEmOrthoWindow::ZFlyEmOrthoWindow(
    const ZDvidEnv &env, int width, int height, int depth, QWidget *parent) :
  QMainWindow(parent)
{
  m_orthoWidget = new ZFlyEmOrthoWidget(env, width, height, depth, this);

  initWidget();
}

void ZFlyEmOrthoWindow::initWidget()
{
  setCentralWidget(m_orthoWidget);
  connect(m_orthoWidget, SIGNAL(bookmarkEdited(int,int,int)),
          this, SIGNAL(bookmarkEdited(int,int,int)));
  connect(m_orthoWidget, SIGNAL(synapseEdited(int,int,int)),
          this, SIGNAL(synapseEdited(int,int,int)));
  connect(m_orthoWidget, SIGNAL(todoEdited(int,int,int)),
          this, SIGNAL(todoEdited(int, int, int)));
  connect(m_orthoWidget, SIGNAL(synapseVerified(int,int,int,bool)),
          this, SIGNAL(synapseVerified(int,int,int,bool)));

  connect(m_orthoWidget, SIGNAL(zoomingTo(int,int,int)),
          this, SIGNAL(zoomingTo(int,int,int)));
  connect(m_orthoWidget, SIGNAL(bodyMergeEdited()),
          this, SIGNAL(bodyMergeEdited()));
  statusBar()->showMessage("Orthogonal view ready.");

  m_progressDlg = NULL;
  m_progressSignal = NULL;
}

void ZFlyEmOrthoWindow::updateData(const ZIntPoint &center)
{
  m_orthoWidget->moveTo(center);
}

void ZFlyEmOrthoWindow::downloadBookmark(int x, int y, int z)
{
  getDocument()->downloadBookmark(x, y, z);
}

void ZFlyEmOrthoWindow::downloadSynapse(int x, int y, int z)
{
  getDocument()->downloadSynapse(x, y, z);
}

void ZFlyEmOrthoWindow::downloadTodo(int x, int y, int z)
{
  getDocument()->downloadTodo(x, y, z);
}

ZFlyEmOrthoDoc *ZFlyEmOrthoWindow::getDocument() const
{
  return m_orthoWidget->getDocument();
}
void ZFlyEmOrthoWindow::copyBookmarkFrom(ZFlyEmProofDoc *doc)
{
  getDocument()->copyBookmarkFrom(doc);
}

void ZFlyEmOrthoWindow::syncBodyColorMap(ZFlyEmProofDoc *doc)
{
  getDocument()->updateBodyColor(doc->getActiveBodyColorMap(), false);
  getDocument()->setActiveBodyColorMap(doc->getActiveBodyColorMap());
}

void ZFlyEmOrthoWindow::syncMergeWithDvid()
{
  m_orthoWidget->syncMergeWithDvid();
}

void ZFlyEmOrthoWindow::processMessage(const ZWidgetMessage &message)
{
  m_orthoWidget->processMessage(message);
}

/*
void ZFlyEmOrthoWindow::closeEvent(QCloseEvent *event)
{
}
*/
