#include "zflyembodysplitproject.h"
#include "zstackframe.h"
#include "z3dwindow.h"
#include "zstackdoclabelstackfactory.h"

ZFlyEmBodySplitProject::ZFlyEmBodySplitProject(QObject *parent) :
  QObject(parent),
  m_bodyId(-1), m_dataFrame(NULL), m_resultWindow(NULL)
{
}

ZFlyEmBodySplitProject::~ZFlyEmBodySplitProject()
{
  clear();
}

void ZFlyEmBodySplitProject::clear()
{
  if (m_resultWindow != NULL) {
    m_resultWindow->hide();
    delete m_resultWindow;
  }

  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
  }

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClearDataFrame()
{
  if (m_resultWindow != NULL) {
    m_resultWindow->hide();
    delete m_resultWindow;
  }

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClear()
{
  m_resultWindow = NULL;
  m_dataFrame = NULL;

  m_bodyId = -1;
}

void ZFlyEmBodySplitProject::shallowClearResultWindow()
{
  m_resultWindow = NULL;
}

void ZFlyEmBodySplitProject::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

void ZFlyEmBodySplitProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
  }
}

void ZFlyEmBodySplitProject::showDataFrame3d()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->open3DWindow(m_dataFrame);
  }
}

void ZFlyEmBodySplitProject::showResult3d()
{
  if (m_dataFrame != NULL) {
    if (m_resultWindow == NULL) {
      ZStackDocReader docReader;
      ZStackDocLabelStackFactory *factory = new ZStackDocLabelStackFactory;
      factory->setDocument(m_dataFrame->document().get());
      ZStack *labeled = factory->makeStack();
      if (labeled != NULL) {
        docReader.setStack(labeled);

        ZStackFrame *newFrame = new ZStackFrame;
        newFrame->addDocData(docReader);
        newFrame->document()->setTag(NeuTube::Document::FLYEM_BODY);
        newFrame->document()->setStackFactory(factory);
        m_dataFrame->connect(
              m_dataFrame->document().get(), SIGNAL(labelFieldModified()),
              newFrame->document().get(), SLOT(reloadStack()));
        m_resultWindow = newFrame->open3DWindow(NULL);
        connect(m_resultWindow, SIGNAL(destroyed()),
                this, SLOT(shallowClearResultWindow()));
        delete newFrame;
      }
    }
    m_resultWindow->show();
    m_resultWindow->raise();
  }
}

bool ZFlyEmBodySplitProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmBodySplitProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    clear();
  }

  m_dataFrame = frame;
}
