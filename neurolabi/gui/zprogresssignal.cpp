#include "zprogresssignal.h"

int ZProgressSignal::m_currentLevel = 0;

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
            target, SIGNAL(progressAdvanced(double)));
    connect(source, SIGNAL(progressStarted(QString)),
            target, SIGNAL(progressStarted(QString)));
    connect(source, SIGNAL(progressEnded()), target, SIGNAL(progressEnded()));
    connect(source, SIGNAL(progressStarted(QString,int)),
            target, SIGNAL(progressStarted(QString,int)));
    connect(source, SIGNAL(progressStarted()),
            target, SIGNAL(progressStarted()));
  }
}

void ZProgressSignal::advanceProgress(double dp)
{
  emit progressAdvanced(dp);
}

void ZProgressSignal::startProgress(const QString &title)
{
  emit progressStarted(title);
}

void ZProgressSignal::startProgress()
{
  emit progressStarted();
}

void ZProgressSignal::startProgress(const QString &title, int nticks)
{
  emit progressStarted(title, nticks);
}

void ZProgressSignal::endProgress()
{
  emit progressEnded();
}
