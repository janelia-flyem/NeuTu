#include "zqtbarprogressreporter.h"

#include <QApplication>

#include "common/math.h"

ZQtBarProgressReporter::ZQtBarProgressReporter()
{
  m_progressBar = NULL;
}

ZQtBarProgressReporter::~ZQtBarProgressReporter()
{

}

void ZQtBarProgressReporter::open()
{
  if (m_progressBar != NULL) {
    m_progressBar->setRange(0, 100);
    m_progressBar->show();
  }
}

void ZQtBarProgressReporter::close()
{
  if (m_progressBar != NULL) {
    m_progressBar->reset();
    m_progressBar->hide();
  }
}

void ZQtBarProgressReporter::push()
{
  m_progressBar->setValue(neutu::iround(m_progress * 100.0));
  //QApplication::processEvents(QEventLoop::AllEvents);
}

void ZQtBarProgressReporter::pull()
{
  m_progress = 0.01 * m_progressBar->value();
}

