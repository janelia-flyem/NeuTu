#ifndef ZFLYEMNEURONFILTERTASKMANAGER_H
#define ZFLYEMNEURONFILTERTASKMANAGER_H

#include <QVector>
#include "zmultitaskmanager.h"
#include "ztask.h"

class ZFlyEmNeuron;
class ZFlyEmNeuronFilter;

class ZFlyEmNeuronFilterTask : public ZTask
{
  Q_OBJECT

public:
  ZFlyEmNeuronFilterTask();

  inline void setTest(ZFlyEmNeuron *neuron) {
    m_testNeuron = neuron;
  }

  inline ZFlyEmNeuron* getResult() const {
    return m_result;
  }

  inline void setFilter(const ZFlyEmNeuronFilter *filter) {
    m_filter = filter;
  }

  void execute();

private:
  ZFlyEmNeuron *m_testNeuron;
  const ZFlyEmNeuronFilter *m_filter;
  ZFlyEmNeuron *m_result;
};

class ZFlyEmNeuronFilterTaskManager : public ZMultiTaskManager
{
public:
  ZFlyEmNeuronFilterTaskManager(QObject *parent = NULL);

  const QVector<ZFlyEmNeuron*>& getResult() {
    return m_filterResult;
  }

protected:
  void prepare();
  void postProcess();

private:
  QVector<ZFlyEmNeuron*> m_filterResult;
};

#endif // ZFLYEMNEURONFILTERTASKMANAGER_H
