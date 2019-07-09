#include "zstackviewrecorder.h"

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
  if (view) {
    if (!m_prefix.isEmpty()) {
      view->takeScreenshot(
            m_prefix + QString::number(m_currentIndex).rightJustified(5, '0'));
    }
  }
}
