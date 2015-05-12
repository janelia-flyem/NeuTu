#ifndef ZPROGRESSSIGNAL_H
#define ZPROGRESSSIGNAL_H

#include <QObject>

class ZProgressSignal : public QObject
{
  Q_OBJECT
public:
  explicit ZProgressSignal(QObject *parent = 0);

  void connectProgress(const ZProgressSignal *signal);

signals:
  void progressStarted(const QString &title, int nticks);
  void progressStarted(const QString &title);
  void progressAdvanced(double dp);
  void progressEnded();

public slots:

};

#endif // ZPROGRESSSIGNAL_H
