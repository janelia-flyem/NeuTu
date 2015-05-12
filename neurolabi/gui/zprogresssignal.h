#ifndef ZPROGRESSSIGNAL_H
#define ZPROGRESSSIGNAL_H

#include <QObject>

class ZProgressSignal : public QObject
{
  Q_OBJECT
public:
  explicit ZProgressSignal(QObject *parent = 0);

  void connectProgress(const ZProgressSignal *signal);
  template <typename T>
  void connectSlot(T *obj);

signals:
  void progressStarted(const QString &title, int nticks);
  void progressStarted(const QString &title);
  void progressAdvanced(double dp);
  void progressEnded();

public slots:
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void advanceProgress(double dp);
  void endProgress();

};

template<typename T>
void ZProgressSignal::connectSlot(T *obj)
{
  connect(this, SIGNAL(progressStarted(QString)),
          obj, SLOT(startProgress(QString)));
  connect(this, SIGNAL(progressStarted(QString,int)),
          obj, SLOT(startProgress(QString,int)));
  connect(this, SIGNAL(progressEnded()), obj, SLOT(endProgress()));
  connect(this, SIGNAL(progressAdvanced(double)),
          obj, SLOT(advanceProgress(double)));
}

#endif // ZPROGRESSSIGNAL_H
