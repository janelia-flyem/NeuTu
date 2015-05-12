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
