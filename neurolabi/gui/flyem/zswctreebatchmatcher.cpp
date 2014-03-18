#include "zswctreebatchmatcher.h"

ZSwcTreeMatchThread::ZSwcTreeMatchThread(QObject *parent) : QThread(parent),
  m_score(0.0), m_source(NULL), m_target(NULL)
{
}

void ZSwcTreeMatchThread::run()
{
  if (m_source != NULL && m_target != NULL) {
#ifdef _DEBUG_
    std::cout << "Matching " << m_source->getId() << ' ' << m_target->getId()
              << std::endl;
#endif
    m_matcher.matchAllG(*m_source->getResampleBuddyModel(),
                        *m_target->getResampleBuddyModel());
    m_score = m_matcher.matchingScore();
  }
}

int ZSwcTreeMatchThread::getSourceId() const
{
  return m_source->getId();
}

int ZSwcTreeMatchThread::getTargetId() const
{
  return m_target->getId();
}

////////////////////////ZFlyEmNeuronLayerMatcher//////////////////
ZFlyEmNeuronMatchThread::ZFlyEmNeuronMatchThread(QObject *parent) :
  QThread(parent), m_score(0.0), m_source(NULL), m_target(NULL)
{
}

void ZFlyEmNeuronMatchThread::run()
{
  if (m_source != NULL && m_target != NULL) {
#ifdef _DEBUG_
    std::cout << "Matching " << m_source->getId() << ' ' << m_target->getId()
              << std::endl;
#endif
    m_score = m_matcher.match(m_source, m_target);
  }
}

int ZFlyEmNeuronMatchThread::getSourceId() const
{
  return m_source->getId();
}

int ZFlyEmNeuronMatchThread::getTargetId() const
{
  return m_target->getId();
}
//////////////////////////////////////////////////////////////////

ZSwcTreeBatchMatcher::ZSwcTreeBatchMatcher(QObject *parent) : QObject(parent),
  m_isFinished(false), m_isStarted(false), m_dataBundle(NULL),
  m_currentIndex(0), m_resampleStep(200.0), m_sourceNeuron(NULL),
  m_trunkAnalyzer(NULL),
  m_featureAnalyzer(NULL), m_helperFeatureAnalyzer(NULL)
{
}

ZSwcTreeBatchMatcher::~ZSwcTreeBatchMatcher()
{
//  delete m_trunkAnalyzer;
//  delete m_featureAnalyzer;
//  delete m_helperFeatureAnalyzer;
}

void ZSwcTreeBatchMatcher::setThreadNumber(int n)
{
  m_threadArray.resize(n);
  for (int i = 0; i < n; ++i) {
    m_threadArray[i] = new ZFlyEmNeuronMatchThread(this);
    connect(m_threadArray[i], SIGNAL(finished()), this, SLOT(process()));
  }
}

void ZSwcTreeBatchMatcher::process()
{
  m_isFinished = true;
  if (!m_isStarted) {
    startProgress();
    foreach (ZFlyEmNeuronMatchThread *thread, m_threadArray) {
      ZFlyEmNeuron *neuron = dequeueNeuron();
      if (neuron != NULL) {
        thread->setTarget(neuron);
        thread->start();
        m_isFinished = false;
      }
    }
    m_isStarted = true;
  } else {
    foreach (ZFlyEmNeuronMatchThread *thread, m_threadArray) {
      if (thread->isFinished()) {
        advanceProgress(1.0 / m_dataBundle->getNeuronArray().size());
        m_result[thread->getTargetId()] = thread->getScore();
        ZFlyEmNeuron *neuron = dequeueNeuron();
        if (neuron != NULL) {
          thread->setTarget(neuron);
          thread->start();
          m_isFinished = false;
        }
      } else {
        m_isFinished = false;
      }
    }
  }

  if (isFinished()) {
    updateTopMatch();
    m_isStarted = false;
    m_currentIndex = 0;
    endProgress();
    emit finished();
  }
}

void ZSwcTreeBatchMatcher::prepare(int threadNumber)
{
  std::vector<ZFlyEmNeuron> &neuronArray = m_dataBundle->getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    neuron.getBody();
//    neuron.getModel();
//    neuron.getResampleBuddyModel(m_resampleStep);

  }

  setThreadNumber(threadNumber);

  foreach (ZFlyEmNeuronMatchThread *thread, m_threadArray) {
    thread->setSource(m_sourceNeuron);
  }

  m_isStarted = false;
}

ZFlyEmNeuron* ZSwcTreeBatchMatcher::dequeueNeuron()
{
  if (m_currentIndex >= (int) m_dataBundle->getNeuronArray().size()) {
    return NULL;
  }

  ZFlyEmNeuron *neuron = &(m_dataBundle->getNeuronArray()[m_currentIndex++]);

  if (neuron == m_sourceNeuron) {
    ++m_currentIndex;
    neuron = &(m_dataBundle->getNeuronArray()[m_currentIndex]);
  }

  return neuron;
}

void ZSwcTreeBatchMatcher::updateTopMatch() const
{
  QVector<const ZFlyEmNeuron*> topMatch;
  double bestScore = 0.0;
  int bestKey = -1;
  for (QMap<int, double>::const_iterator iter = m_result.begin();
       iter != m_result.end(); ++iter) {
    if (bestScore < iter.value()) {
      bestKey = iter.key();
      bestScore = iter.value();
    }
  }

  topMatch.append(m_dataBundle->getNeuron(bestKey));

  m_sourceNeuron->setMatched(topMatch.begin(), topMatch.end());
}

void ZSwcTreeBatchMatcher::setSourceNeuron(int id)
{
  m_sourceNeuron = m_dataBundle->getNeuron(id);
}
