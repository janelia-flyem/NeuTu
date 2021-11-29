#include "zstackviewrecorder.h"

#include "common/debug.h"
#include "zstackview.h"

ZStackViewRecorder::ZStackViewRecorder()
{

}


void ZStackViewRecorder::restart()
{
  m_currentIndex = 1;
}

void ZStackViewRecorder::takeShot(ZStackView *view)
{
  if (view && !m_prefix.isEmpty()) {
    bool recording = false;
    if (!isAuto()) {
      recording = true;
    } else if (m_lastRecordedFrame.count(view->getViewId()) == 0) {
      recording = true;
    } else if (m_lastRecordedFrame.value(view->getViewId())
               < view->getFrameCount()) {
      recording = true;
    }
    if (recording) {
#ifdef _DEBUG_0
      HLDebug("2D View") << "Start recording frame " << view->getFrameCount() << std::endl;
#endif
      QString filepath = m_prefix +
          QString::number(m_currentIndex++).rightJustified(5, '0') + ".png";
      view->takeScreenshot(filepath);
      m_lastRecordedFrame[view->getViewId()] = view->getFrameCount();
#ifdef _DEBUG_0
      HLDebug("2D View") << filepath.toStdString() << " saved." << std::endl;
#endif
    }
  }
}

bool ZStackViewRecorder::isAuto() const
{
  return m_mode == EMode::AUTO;
}

void ZStackViewRecorder::setPrefix(const QString prefix)
{
  m_prefix = prefix;
}

void ZStackViewRecorder::setMode(EMode mode)
{
  m_mode = mode;
}

QString ZStackViewRecorder::getPrefix() const
{
  return m_prefix;
}
