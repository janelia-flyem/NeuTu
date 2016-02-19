#include "zflyemneuronmatchtaskmanager.h"
#include "zflyemdatabundle.h"
#include "zflyemneuron.h"
#include "zobject3dscan.h"
#include "zswctree.h"

////////////////////////ZFlyEmNeuronLayerMatcher//////////////////
ZFlyEmNeuronMatchTask::ZFlyEmNeuronMatchTask(QObject *parent) :
  ZTask(parent), m_score(0.0), m_source(NULL), m_target(NULL)
{
  m_matcher.setLayerScale(1.0);
}

void ZFlyEmNeuronMatchTask::execute()
{
  if (m_source != NULL && m_target != NULL) {
#ifdef _DEBUG_
    std::cout << "Matching " << m_source->getId() << ' ' << m_target->getId()
              << std::endl;
#endif
    m_score = m_matcher.match(m_source, m_target);
  }
}

int ZFlyEmNeuronMatchTask::getSourceId() const
{
  return m_source->getId();
}

int ZFlyEmNeuronMatchTask::getTargetId() const
{
  return m_target->getId();
}

void ZFlyEmNeuronMatchTask::prepare()
{
  if (m_source != NULL) {
    ZObject3dScan *body = m_source->getBody();
    if (body != NULL) {
      body->getSlicewiseVoxelNumber();
    }
    ZSwcTree *tree = m_source->getModel();
    if (tree != NULL) {
      tree->getBoundBox();
    }
  }

  if (m_target != NULL) {
    ZObject3dScan *body = m_target->getBody();
    if (body != NULL) {
      body->getSlicewiseVoxelNumber();
    }
    ZSwcTree *tree = m_target->getModel();
    if (tree != NULL) {
      tree->getBoundBox();
    }
  }
}

void ZFlyEmNeuronMatchTask::setLayerScale(double scale)
{
  m_matcher.setLayerScale(scale);
}

//////////////////////////////////////////////////////////////////


ZFlyEmNeuronMatchTaskManager::ZFlyEmNeuronMatchTaskManager(QObject *parent) :
  ZMultiTaskManager(parent)
{
}

void ZFlyEmNeuronMatchTaskManager::prepare()
{
  m_result.clear();
}

void ZFlyEmNeuronMatchTaskManager::postProcess()
{
  if (!m_taskArray.empty()) {
    foreach (ZTask *task, m_taskArray) {
      ZFlyEmNeuronMatchTask *matchTask =
          qobject_cast<ZFlyEmNeuronMatchTask*>(task);
      if (matchTask != NULL) {
        ZFlyEmNeuron *source = matchTask->getSource();
        ZFlyEmNeuron *target = matchTask->getTarget();
        if (source != NULL && target != NULL) {
          if (!m_result.contains(source)) {
            QMap<ZFlyEmNeuron*, double> scoreMap;
            m_result[source] = scoreMap;
          }
          m_result[source][target] = matchTask->getScore();
        }
      }
    }

    foreach (ZFlyEmNeuron *source, m_result.keys()) {
      QVector<const ZFlyEmNeuron*> topMatch;
      double bestScore = 0.0;
      ZFlyEmNeuron *bestKey = NULL;
      QMap<ZFlyEmNeuron*, double> &scoreMap = m_result[source];
      for (QMap<ZFlyEmNeuron*, double>::const_iterator iter = scoreMap.begin();
           iter != scoreMap.end(); ++iter) {
        if (bestScore <= iter.value()) {
          bestKey = iter.key();
          bestScore = iter.value();
        }
      }

      topMatch.append(bestKey);

      source->setMatched(topMatch.begin(), topMatch.end());
    }
  }
}
