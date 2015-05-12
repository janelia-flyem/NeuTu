#include "zprogresssignal.h"

ZProgressSignal::ZProgressSignal(QObject *parent) :
  QObject(parent)
{
}

void ZProgressSignal::connectProgress(const ZProgressSignal *signal)
{
  connect(this, SIGNAL(progressAdvanced(double)),
          signal, SIGNAL(progressAdvanced(double)));
  connect(this, SIGNAL(progressStarted(QString)),
          signal, SIGNAL(progressStarted(QString)));
  connect(this, SIGNAL(progressEnded()), this, SIGNAL(progressEnded()));
  connect(this, SIGNAL(progressStarted(QString,int)),
          signal, SIGNAL(progressStarted(QString,int)));
}

void ZProgressSignal::advanceProgress(double dp)
{
  emit progressAdvanced(dp);
}

void ZProgressSignal::startProgress(const QString &title)
{
  emit progressStarted(title);
}

void ZProgressSignal::startProgress(const QString &title, int nticks)
{
  emit progressStarted(title, nticks);
}

void ZProgressSignal::endProgress()
{
  emit progressEnded();
}
