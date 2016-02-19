#include "zprogresssignal.h"

//int ZProgressSignal::m_currentLevel = 0;

ZProgressSignal::ZProgressSignal(QObject *parent) :
  QObject(parent)
{
}

void ZProgressSignal::connectProgress(const ZProgressSignal *targetSignal)
{
  ConnectProgress(this, targetSignal);
}

void ZProgressSignal::ConnectProgress(
    const ZProgressSignal *source, const ZProgressSignal *target)
{
  if (source != NULL && target != NULL) {
    connect(source, SIGNAL(progressAdvanced(double)),
            target, SLOT(advanceProgress(double)));
    connect(source, SIGNAL(progressStarted(QString)),
            target, SLOT(startProgress(QString)));
    connect(source, SIGNAL(progressEnded()), target, SLOT(endProgress()));
    connect(source, SIGNAL(progressStarted(QString,int)),
            target, SLOT(startProgress(QString,int)));
    connect(source, SIGNAL(progressStarted()),
            target, SLOT(startProgress()));
  }
}

void ZProgressSignal::advanceProgress(double dp)
{
  emit progressAdvanced(dp * getSubFactor());
}

void ZProgressSignal::startProgress(const QString &title)
{
  if (m_subStack.isEmpty()) {
    emit progressStarted(title);
  }
}

void ZProgressSignal::startProgress()
{
  if (m_subStack.isEmpty()) {
    emit progressStarted();
  }
}

void ZProgressSignal::startProgress(const QString &title, int nticks)
{
  if (m_subStack.isEmpty()) {
    emit progressStarted(title, nticks);
  }
}

void ZProgressSignal::endProgress()
{
  if (!m_subStack.isEmpty()) {
    m_subStack.pop();
  } else {
    emit progressEnded();
  }
}

void ZProgressSignal::startProgress(double alpha)
{
  m_subStack.push(alpha);
}

double ZProgressSignal::getSubFactor() const
{
  double alpha = 1.0;
  for (QStack<double>::const_iterator iter = m_subStack.begin();
       iter != m_subStack.end(); ++iter) {
    alpha *= *iter;
  }

  return alpha;
}
