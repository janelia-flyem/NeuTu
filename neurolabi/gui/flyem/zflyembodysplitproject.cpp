#include "zflyembodysplitproject.h"
#include "zstackframe.h"
#include "z3dwindow.h"
#include "zstackdoclabelstackfactory.h"
#include "zstackobject.h"
#include "zcircle.h"
#include "zsparsestack.h"
#include "z3dvolumesource.h"

ZFlyEmBodySplitProject::ZFlyEmBodySplitProject(QObject *parent) :
  QObject(parent), m_bodyId(-1), m_dataFrame(NULL), m_resultWindow(NULL),
  m_isBookmarkVisible(true)
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

  m_bookmarkDecoration.clear();

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
        newFrame->document()->setTag(NeuTube::Document::FLYEM_SPLIT);
        newFrame->document()->setStackFactory(factory);
        m_dataFrame->connect(
              m_dataFrame->document().get(), SIGNAL(labelFieldModified()),
              newFrame->document().get(), SLOT(reloadStack()));
        m_resultWindow = newFrame->open3DWindow(NULL);
        if (m_dataFrame->document()->hasSparseStack()) {
          ZIntPoint dsIntv =
              m_dataFrame->document()->getSparseStack()->getDownsampleInterval();
          if (dsIntv.getX() != dsIntv.getZ()) {
            m_resultWindow->getVolumeSource()->setZScale(
                  ((float) (dsIntv.getZ() + 1)) / (dsIntv.getX() + 1));
            m_resultWindow->resetCamera();
          }
        }
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
  updateBookDecoration();
}

void ZFlyEmBodySplitProject::loadBookmark(const QString &filePath)
{
  m_bookmarkArray.importJsonFile(filePath.toStdString());
}

void ZFlyEmBodySplitProject::locateBookmark(const ZFlyEmBookmark &bookmark)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->viewRoi(bookmark.getLocation().getX(),
                         bookmark.getLocation().getY(),
                         bookmark.getLocation().getZ(), 5);
  }
}

void ZFlyEmBodySplitProject::clearBookmarkDecoration()
{
  if (m_dataFrame != NULL) {
    for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
         iter != m_bookmarkDecoration.end(); ++iter) {
      ZStackObject *obj = *iter;
      m_dataFrame->document()->removeObject(obj, false);
      delete obj;
    }
  } else {
    for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
         iter != m_bookmarkDecoration.end(); ++iter) {
      delete *iter;
    }
  }
  m_bookmarkDecoration.clear();
}

void ZFlyEmBodySplitProject::addBookmarkDecoration(
    const ZFlyEmBookmarkArray &bookmarkArray)
{
  if (m_dataFrame != NULL) {
    for (ZFlyEmBookmarkArray::const_iterator iter = bookmarkArray.begin();
         iter != bookmarkArray.end(); ++iter) {
      const ZFlyEmBookmark &bookmark = *iter;
      ZCircle *circle = new ZCircle;
      circle->set(bookmark.getLocation(), 5);
      circle->setColor(255, 0, 0);
      circle->setVisible(m_isBookmarkVisible);
      m_dataFrame->document()->addObject(circle);
      m_bookmarkDecoration.push_back(circle);
    }
  }
}

void ZFlyEmBodySplitProject::updateBookDecoration()
{
  clearBookmarkDecoration();

  if (m_dataFrame != NULL) {
    ZFlyEmBookmarkArray bookmarkArray;
    foreach (ZFlyEmBookmark bookmark, m_bookmarkArray) {
      if (bookmark.getBodyId() == getBodyId()) {
        bookmarkArray.append(bookmark);
      }
    }
    addBookmarkDecoration(bookmarkArray);
  }
}

void ZFlyEmBodySplitProject::showBookmark(bool visible)
{
  m_isBookmarkVisible = visible;
  for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
       iter != m_bookmarkDecoration.end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setVisible(visible);
  }
  if (m_dataFrame != NULL && !m_bookmarkDecoration.empty()) {
    m_dataFrame->updateView();
  }
}

std::set<int> ZFlyEmBodySplitProject::getBookmarkBodySet() const
{
  std::set<int> bodySet;
  foreach (ZFlyEmBookmark bookmark, m_bookmarkArray) {
    bodySet.insert(bookmark.getBodyId());
  }

  return bodySet;
}
