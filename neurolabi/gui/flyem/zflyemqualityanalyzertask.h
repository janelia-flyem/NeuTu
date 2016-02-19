#ifndef ZFLYEMQUALITYANALYZERTASK_H
#define ZFLYEMQUALITYANALYZERTASK_H

#include "ztask.h"
#include "flyem/zhotspotarray.h"

class ZFlyEmNeuron;
class ZFlyEmDataBundle;

class ZFlyEmQualityAnalyzerTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZFlyEmQualityAnalyzerTask(QObject *parent = 0);

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

#endif // ZFLYEMQUALITYANALYZERTASK_H
