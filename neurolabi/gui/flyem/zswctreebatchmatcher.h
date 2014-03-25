#ifndef ZSWCTREEBATCHMATCHER_H
#define ZSWCTREEBATCHMATCHER_H

#include <QThread>
#include <QMap>
#include <QVector>
#include <QRunnable>
#include "zflyemdatabundle.h"
#include "zswctreematcher.h"
#include "zflyemdatabundle.h"
#include "flyem/zflyemneuronlayermatcher.h"
#include "zprogressable.h"

class ZSwcTreeMatchThread : public QThread
{
public:
  ZSwcTreeMatchThread(QObject *parent);
  void run();

  int getSourceId() const;
  int getTargetId() const;
  inline double getScore() const { return m_score; }

  inline void setSource(ZFlyEmNeuron *source) {
    m_source = source;
  }
  inline void setTarget(ZFlyEmNeuron *target) {
    m_target = target;
  }


private:
  double m_score;
  ZFlyEmNeuron *m_source;
  ZFlyEmNeuron *m_target;
  ZSwcTreeMatcher m_matcher;
};

class ZFlyEmNeuronMatchThread : public QThread
{
public:
  ZFlyEmNeuronMatchThread(QObject *parent);
  void run();

  int getSourceId() const;
  int getTargetId() const;
  inline double getScore() const { return m_score; }

  inline void setSource(ZFlyEmNeuron *source) {
    m_source = source;
  }
  inline void setTarget(ZFlyEmNeuron *target) {
    m_target = target;
  }


private:
  double m_score;
  ZFlyEmNeuron *m_source;
  ZFlyEmNeuron *m_target;
  ZFlyEmNeuronLayerMatcher m_matcher;
};
/*
class ZFlyEmNeuronMatchTask : public QObject, public QRunnable
{
  Q_OBJECT

public:
  ZFlyEmNeuronMatchTask(QObject *parent);

};
*/

class ZSwcTreeBatchMatcher : public QObject, public ZProgressable
{
  Q_OBJECT

public:
  ZSwcTreeBatchMatcher(QObject *parent);
  ~ZSwcTreeBatchMatcher();
  void setThreadNumber(int n);
  void prepare(int threadNumber);
  ZFlyEmNeuron* dequeueNeuron();
  inline bool isFinished() const { return m_isFinished; }

  void updateTopMatch() const;
  inline int getSourceId() const { return m_sourceNeuron->getId(); }
  inline ZFlyEmNeuron* getSourceNeuron() const { return m_sourceNeuron; }
  inline void setDataBundle(ZFlyEmDataBundle *bundle) {
    m_dataBundle = bundle;
  }
  inline void setResampleStep(double step) {
    m_resampleStep = step;
  }
  void setSourceNeuron(int id);


public slots:
  void process();

signals:
  void finished();

private:
  QMap<int, double> m_result;
  bool m_isFinished;
  bool m_isStarted;
  QVector<ZFlyEmNeuronMatchThread*> m_threadArray;
  ZFlyEmDataBundle *m_dataBundle;
  int m_currentIndex;
  double m_resampleStep;
  ZFlyEmNeuron *m_sourceNeuron;

  ZSwcTrunkAnalyzer *m_trunkAnalyzer;
  ZSwcFeatureAnalyzer *m_featureAnalyzer;
  ZSwcFeatureAnalyzer *m_helperFeatureAnalyzer;
};

#endif // ZSWCTREEBATCHMATCHER_H
