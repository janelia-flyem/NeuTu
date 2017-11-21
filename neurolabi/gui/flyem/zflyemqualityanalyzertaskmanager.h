#ifndef ZFLYEMQUALITYANALYZERTASKMANAGER_H
#define ZFLYEMQUALITYANALYZERTASKMANAGER_H

#include "zmultitaskmanager.h"
#include "flyem/zhotspotarray.h"
#include "ztask.h"

class ZFlyEmQualityAnalyzerTaskManager : public ZMultiTaskManager
{
  Q_OBJECT

public:
  ZFlyEmQualityAnalyzerTaskManager(QObject *parent = NULL);

  flyem::ZHotSpotArray& getHotSpot() { return m_hotSpotArray; }

protected:
  virtual void prepare();
  virtual void postProcess();

private:
  flyem::ZHotSpotArray m_hotSpotArray;
};

#endif // ZFLYEMQUALITYANALYZERTASKMANAGER_H
