#include "zflyemorthowindow.h"

#include <QStatusBar>

#include "flyem/zflyemorthowidget.h"
#include "flyem/zflyemorthodoc.h"

ZFlyEmOrthoWindow::ZFlyEmOrthoWindow(const ZDvidTarget &target, QWidget *parent) :
  QMainWindow(parent)
{
  m_orthoWidget = new ZFlyEmOrthoWidget(target, this);

  initWidget();
}

ZFlyEmOrthoWindow::ZFlyEmOrthoWindow(
    const ZDvidTarget &target, int width, int height, int depth, QWidget *parent) :
  QMainWindow(parent)
{
  m_orthoWidget = new ZFlyEmOrthoWidget(target, width, height, depth, this);

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
  getDocument()->updateBodyColor(doc->getActiveBodyColorMap());
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
