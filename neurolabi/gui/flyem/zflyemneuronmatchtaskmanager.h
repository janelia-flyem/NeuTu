#ifndef ZFLYEMNEURONMATCHTASKMANAGER_H
#define ZFLYEMNEURONMATCHTASKMANAGER_H

#include <QMap>
#include <QVector>
#include "zmultitaskmanager.h"
#include "flyem/zflyemneuronlayermatcher.h"
#include "ztask.h"

class ZFlyEmNeuron;
class ZFlyEmDataBundle;

class ZFlyEmNeuronMatchTask : public ZTask
{
  Q_OBJECT

public:
  ZFlyEmNeuronMatchTask(QObject *parent = NULL);

  void execute();

  int getSourceId() const;
  int getTargetId() const;
  inline double getScore() const { return m_score; }

  inline ZFlyEmNeuron* getSource() const {
    return m_source;
  }

  inline ZFlyEmNeuron* getTarget() const {
    return m_target;
  }

  inline void setSource(ZFlyEmNeuron *source) {
    m_source = source;
  }
  inline void setTarget(ZFlyEmNeuron *target) {
    m_target = target;
  }

  void prepare();

  void setLayerScale(double scale);

private:
  double m_score;
  ZFlyEmNeuron *m_source;
  ZFlyEmNeuron *m_target;
  ZFlyEmNeuronLayerMatcher m_matcher;
};


class ZFlyEmNeuronMatchTaskManager : public ZMultiTaskManager
{
  Q_OBJECT

public:
  ZFlyEmNeuronMatchTaskManager(QObject *parent = NULL);

protected:
  virtual void prepare();
  virtual void postProcess();

private:
  QMap<ZFlyEmNeuron*, QMap<ZFlyEmNeuron*, double> >m_result;
  //QVector<ZFlyEmNeuron*> m_sourceNeuronArray;
};


#endif // ZFLYEMNEURONMATCHTASKMANAGER_H
