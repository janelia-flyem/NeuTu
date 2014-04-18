#ifndef ZFLYEMQUALITYANALYZERTASKMANAGER_H
#define ZFLYEMQUALITYANALYZERTASKMANAGER_H

#include "zmultitaskmanager.h"
#include "flyem/zhotspotarray.h"

class ZFlyEmNeuron;
class ZFlyEmDataBundle;

class ZFlyEmQualityAnalyzerTask : public ZTask
{
public:
  ZFlyEmQualityAnalyzerTask(QObject *parent = NULL);

  void execute();

  int getSourceId() const;
  int getTargetId() const;
  inline FlyEm::ZHotSpotArray* getHotSpot() { return &m_hotSpotArray; }

  inline ZFlyEmNeuron* getSource() const {
    return m_source;
  }

  inline void setSource(ZFlyEmNeuron *source) {
    m_source = source;
  }
  inline void setDataBundle(ZFlyEmDataBundle *bundle) {
    m_dataBundle = bundle;
  }

  void prepare();

private:
  ZFlyEmNeuron *m_source;
  ZFlyEmDataBundle *m_dataBundle;
  FlyEm::ZHotSpotArray m_hotSpotArray;
};

class ZFlyEmQualityAnalyzerTaskManager : public ZMultiTaskManager
{
public:
  ZFlyEmQualityAnalyzerTaskManager(QObject *parent = NULL);

  FlyEm::ZHotSpotArray& getHotSpot() { return m_hotSpotArray; }

protected:
  virtual void prepare();
  virtual void postProcess();

private:
  FlyEm::ZHotSpotArray m_hotSpotArray;
};

#endif // ZFLYEMQUALITYANALYZERTASKMANAGER_H
