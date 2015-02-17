#ifndef ZPROGRESSMANAGER_H
#define ZPROGRESSMANAGER_H

#include <QObject>

#include "zprogressreporter.h"

class ZProgressManager : public QObject
{
  Q_OBJECT
public:
  explicit ZProgressManager(QObject *parent = 0);
  virtual ~ZProgressManager();

public:
  void setProgressReporter(ZProgressReporter *reporter);
  void setDefaultProgressReporter();
  void destroyProgressReporter();
  inline ZProgressReporter* getProgressReporter() { return m_progressReporter; }

  void notifyProgressStarted();
  void notifyProgressEnded();
  void notifyProgressStarted(double scale);
  void notifyProgressEnded(double scale);
  void notifyProgressAdvanced(double dp);
  void notifyProgressReset();

signals:
  void progressStarted();
  void progressEnded();
  void progressStarted(double scale);
  void progressEnded(double scale);
  void progressAdvanced(double dp);
  void progressReset();

public slots:
  void startProgress();
  void endProgress();
  void startProgress(double scale);
  void endProgress(double scale);
  void advanceProgress(double dp);
  void resetProgress();

protected:
  void connectSignalSlot();

protected:
  ZProgressReporter m_defaultProgressReporter;
  ZProgressReporter *m_progressReporter;

};

#endif // ZPROGRESSMANAGER_H
