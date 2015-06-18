#ifndef ZPROGRESSSIGNAL_H
#define ZPROGRESSSIGNAL_H

#include <QObject>
#include <QStack>

class ZProgressSignal : public QObject
{
  Q_OBJECT
public:
  explicit ZProgressSignal(QObject *parent = 0);

  void connectProgress(const ZProgressSignal *targetSignal);
  template <typename T>
  void connectSlot(T *obj);

  static void ConnectProgress(const ZProgressSignal *source,
                              const ZProgressSignal *target);
  /*
  static int getCurrentLevel() {
    return m_currentLevel;
  }
*/
  double getSubFactor() const;

signals:
  void progressStarted(const QString &title, int nticks);
  void progressStarted(const QString &title);
  void progressAdvanced(double dp);
  void progressEnded();
  void progressStarted();

public slots:
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void startProgress();
  void startProgress(double alpha);
  void advanceProgress(double dp);
  void endProgress();

private:
  //static int m_currentLevel;
  QStack<double> m_subStack;
};

template<typename T>
void ZProgressSignal::connectSlot(T *obj)
{
  if (obj != NULL) {
    connect(this, SIGNAL(progressStarted()),
            obj, SLOT(startProgress()));
    connect(this, SIGNAL(progressStarted(QString)),
            obj, SLOT(startProgress(QString)));
    connect(this, SIGNAL(progressStarted(QString,int)),
            obj, SLOT(startProgress(QString,int)));
    connect(this, SIGNAL(progressEnded()), obj, SLOT(endProgress()));
    connect(this, SIGNAL(progressAdvanced(double)),
            obj, SLOT(advanceProgress(double)));
  }
}

#endif // ZPROGRESSSIGNAL_H
