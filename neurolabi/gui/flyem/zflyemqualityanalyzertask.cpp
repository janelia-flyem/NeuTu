#include "zflyemqualityanalyzertask.h"

#include "zflyemdatabundle.h"
#include "zflyemneuron.h"
#include "zswctree.h"
#include "flyem/zflyemqualityanalyzer.h"

ZFlyEmQualityAnalyzerTask::ZFlyEmQualityAnalyzerTask(QObject *parent) :
  ZTask(parent), m_source(NULL), m_dataBundle(NULL)
{

}

void ZFlyEmQualityAnalyzerTask::prepare()
{
  if (m_source != NULL) {
    ZSwcTree *tree = m_source->getModel();
    if (tree != NULL) {
      tree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);
      tree->getBoundBox();
    }
  }

  if (m_dataBundle != NULL) {
    std::vector<ZFlyEmNeuron>& neuronArray = m_dataBundle->getNeuronArray();
    for (size_t i = 0; i < neuronArray.size(); ++i) {
      ZSwcTree *tree = neuronArray[i].getModel();
      if (tree != NULL) {
        tree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);
        tree->getBoundBox();
      }
    }
  }
}

void ZFlyEmQualityAnalyzerTask::execute()
{
  if (m_source != NULL && m_dataBundle != NULL) {
    ZFlyEmQualityAnalyzer analyzer;
    m_hotSpotArray.concat(
          &(analyzer.computeHotSpot(m_source, m_dataBundle->getNeuronArray())));
  }
}
