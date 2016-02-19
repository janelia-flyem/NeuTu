#include "zflyemqualityanalyzertaskmanager.h"
#include "zflyemneuron.h"
#include "zswctree.h"
#include "flyem/zflyemqualityanalyzer.h"
#include "flyem/zflyemqualityanalyzertask.h"

ZFlyEmQualityAnalyzerTaskManager::ZFlyEmQualityAnalyzerTaskManager(
    QObject *parent) : ZMultiTaskManager(parent)
{

}


void ZFlyEmQualityAnalyzerTaskManager::prepare()
{
  m_hotSpotArray.clear();
}

void ZFlyEmQualityAnalyzerTaskManager::postProcess()
{
  if (!m_taskArray.empty()) {
    foreach (ZTask *task, m_taskArray) {
      ZFlyEmQualityAnalyzerTask *analyzerTask =
          qobject_cast<ZFlyEmQualityAnalyzerTask*>(task);
      if (analyzerTask != NULL) {
        m_hotSpotArray.concat(analyzerTask->getHotSpot());
      }
    }
  }
}
