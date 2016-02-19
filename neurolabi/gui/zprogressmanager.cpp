#include "zprogressmanager.h"

ZProgressManager::ZProgressManager(QObject *parent) :
  QObject(parent)
{
  connectSignalSlot();
}

ZProgressManager::~ZProgressManager()
{
}

void ZProgressManager::connectSignalSlot()
{
  connect(this, SIGNAL(progressStarted()), this, SLOT(startProgress()));
  connect(this, SIGNAL(progressEnded()), this, SLOT(endProgress()));
  connect(this, SIGNAL(progressReset()), this, SLOT(resetProgress()));
  connect(this, SIGNAL(progressStarted(double)),
          this, SLOT(startProgress(double)));
  connect(this, SIGNAL(progressEnded(double)),
          this, SLOT(endProgress(double)));
  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgress(double)));
}

void ZProgressManager::destroyProgressReporter()
{
  if (m_progressReporter != &m_defaultProgressReporter) {
    delete m_progressReporter;
    setDefaultProgressReporter();
  }
}

void ZProgressManager::setDefaultProgressReporter()
{
  m_progressReporter = &m_defaultProgressReporter;
}

void ZProgressManager::setProgressReporter(ZProgressReporter *reporter)
{
   if (reporter == 0) {
     setDefaultProgressReporter();
   } else {
     m_progressReporter = reporter;
   }
 }

void ZProgressManager::startProgress()
{
  m_progressReporter->start();
}

void ZProgressManager::endProgress()
{
  m_progressReporter->end();
}

void ZProgressManager::resetProgress()
{
  m_progressReporter->resetSessionCount();
  m_progressReporter->close();
}

void ZProgressManager::startProgress(double scale)
{
  m_progressReporter->start(scale);
}

void ZProgressManager::endProgress(double scale)
{
  m_progressReporter->end(scale);
}

void ZProgressManager::advanceProgress(double dp)
{
  m_progressReporter->advance(dp);
}

void ZProgressManager::notifyProgressStarted()
{
  emit progressStarted();
}

void ZProgressManager::notifyProgressEnded()
{
  emit progressEnded();
}

void ZProgressManager::notifyProgressStarted(double scale)
{
  emit progressStarted(scale);
}

void ZProgressManager::notifyProgressEnded(double scale)
{
  emit progressEnded(scale);
}

void ZProgressManager::notifyProgressAdvanced(double dp)
{
  emit progressAdvanced(dp);
}

void ZProgressManager::notifyProgressReset()
{
  emit progressReset();
}

