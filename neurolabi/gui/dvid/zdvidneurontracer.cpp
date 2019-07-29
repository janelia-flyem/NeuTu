#include "zdvidneurontracer.h"

#include <cmath>

#include "neutubeconfig.h"

#include "geometry/zintcuboid.h"
#include "tz_math.h"
#include "tz_locseg_chain.h"
#include "zswctree.h"
#include "zstack.hxx"
#include "imgproc/zstackprocessor.h"

ZDvidNeuronTracer::ZDvidNeuronTracer()
{
  init();
}

ZDvidNeuronTracer::~ZDvidNeuronTracer()
{
//  delete m_resultTree;
  for (std::list<Local_Neuroseg*>::iterator iter = m_locsegList.begin();
       iter != m_locsegList.end(); ++iter) {
    Kill_Local_Neuroseg(*iter);
  }

  Kill_Locseg_Fit_Workspace(m_fitWorkspace);
}

void ZDvidNeuronTracer::init()
{
//  m_resultTree = NULL;

  m_fitWorkspace = New_Locseg_Fit_Workspace();
  m_fitWorkspace->sws->fs.n = 2;
  m_fitWorkspace->sws->fs.options[0] = STACK_FIT_DOT;
  m_fitWorkspace->sws->fs.options[1] = STACK_FIT_CORRCOEF;

  m_traceMinScore = 0.3;
  m_seedMinScore = 0.3;
}

void ZDvidNeuronTracer::setDvidTarget(const ZDvidTarget &target)
{
  if (m_dvidReader.open(target)) {
    m_dvidInfo = m_dvidReader.readGrayScaleInfo();
  }
}

void ZDvidNeuronTracer::setDvidReader(const ZDvidReader &reader)
{
  m_dvidReader = reader;
  m_dvidInfo = m_dvidReader.readGrayScaleInfo();
}

ZSwcTree* ZDvidNeuronTracer::getResult() const
{
  ZSwcTree *tree = NULL;

  if (!m_locsegList.empty()) {
    Locseg_Chain *chain = New_Locseg_Chain();

    for (std::list<Local_Neuroseg*>::const_iterator iter = m_locsegList.begin();
         iter != m_locsegList.end(); ++iter) {
      Local_Neuroseg *locseg = const_cast<Local_Neuroseg*>(*iter);
      Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
    }

    int n;
    /* alloc <circles> */
    Geo3d_Circle *circles =
        Locseg_Chain_To_Geo3d_Circle_Array(chain, NULL, &n);
    Delete_Locseg_Chain(chain);

    Swc_Tree_Node *root = SwcTreeNode::MakePointer(circles[0].center[0],
        circles[0].center[1], circles[0].center[2], circles[0].radius);
    Swc_Tree_Node *parent =root;
    for (int i = 1; i < n; ++i) {
      Swc_Tree_Node *tn = SwcTreeNode::MakePointer(circles[i].center[0],
          circles[i].center[1], circles[i].center[2], circles[i].radius, parent);
      parent = tn;
    }

    tree = new ZSwcTree;
    tree->setDataFromNode(root);
  }
  return tree;
}

ZStack* ZDvidNeuronTracer::readStack(const ZIntCuboid &box)
{
  ZStack *stack = m_dvidReader.readGrayScale(box);

  ZStackProcessor::SubtractBackground(stack);

#ifdef _DEBUG_2
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

  return stack;
}

ZStack* ZDvidNeuronTracer::readStack(
    double x, double y, double z, double r)
{
  if (!m_dvidInfo.getDataRange().contains(x, y, z)) {
    return NULL;
  }

  r = std::max(r * 3.0, 16.0);

  ZIntCuboid box;
  box.setFirstCorner(std::floor(x - r), std::floor(y - r), std::floor(z - r));
  box.setLastCorner(std::ceil(x + r), std::ceil(y + r), std::ceil(z + r));

  return readStack(box);
}

double ZDvidNeuronTracer::getFitScore() const
{
  return m_fitWorkspace->sws->fs.scores[1];
}

void ZDvidNeuronTracer::registerToRawStack(
    const ZIntPoint &stackOffset, Local_Neuroseg *locseg)
{
  double pos[3];

  Local_Neuroseg_Center(locseg, pos);

  pos[0] -= stackOffset.getX();
  pos[1] -= stackOffset.getY();
  pos[2] -= stackOffset.getZ();

  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);
}

void ZDvidNeuronTracer::registerToStack(
    const ZIntPoint &stackOffset, Local_Neuroseg *locseg)
{
  double pos[3];

  Local_Neuroseg_Center(locseg, pos);

  pos[0] += stackOffset.getX();
  pos[1] += stackOffset.getY();
  pos[2] += stackOffset.getZ();

  Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);
}


double ZDvidNeuronTracer::fit(Local_Neuroseg *locseg)
{
  Geo3d_Ball ball;
  Local_Neuroseg_Ball_Bound(locseg, &ball);

  ZStack *stack = readStack(
        ball.center[0], ball.center[1], ball.center[2], ball.r);

  double score = -1.0;

  if (stack != NULL) {
    registerToRawStack(stack->getOffset(), locseg);
    Fit_Local_Neuroseg_W(locseg, stack->c_stack(), 1.0, m_fitWorkspace);
    registerToStack(stack->getOffset(), locseg);
    score = getFitScore();
  }

  delete stack;

  return score;
}

double ZDvidNeuronTracer::optimize(Local_Neuroseg *locseg)
{
  Geo3d_Ball ball;
  Local_Neuroseg_Ball_Bound(locseg, &ball);

  ZStack *stack = readStack(
        ball.center[0], ball.center[1], ball.center[2], ball.r);

  double score = -1.0;

  if (stack != NULL) {
    Locseg_Fit_Workspace *fws = New_Locseg_Fit_Workspace();
    fws->sws->fs.n = 2;
    fws->sws->fs.options[0] = STACK_FIT_DOT;
    fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;
    fws->pos_adjust = 1;

    registerToRawStack(stack->getOffset(), locseg);
    Local_Neuroseg_Optimize_W(locseg, stack->c_stack(), 1.0, 0, fws);
    fws->pos_adjust = 0;
    Flip_Local_Neuroseg(locseg);
    Fit_Local_Neuroseg_W(locseg, stack->c_stack(), 1.0, fws);
    Flip_Local_Neuroseg(locseg);
    Fit_Local_Neuroseg_W(locseg, stack->c_stack(), 1.0, fws);

    registerToStack(stack->getOffset(), locseg);
    score = fws->sws->fs.scores[1];

    Kill_Locseg_Fit_Workspace(fws);
  }

  delete stack;

  return score;
}

bool ZDvidNeuronTracer::hitTraced(const Local_Neuroseg *locseg) const
{
  bool hit = false;

  double top[3];
  Local_Neuroseg_Top(locseg, top);
  for (std::list<Local_Neuroseg*>::const_iterator iter = m_locsegList.begin();
       iter != m_locsegList.end(); ++iter) {
    const Local_Neuroseg *locseg = *iter;
    if (Local_Neuroseg_Hit_Test(locseg, top[0], top[1], top[2])) {
      hit = true;
      break;
    }
  }

  return hit;
}

void ZDvidNeuronTracer::trace(double x, double y, double z, double r)
{
  m_locsegList.clear();

  Local_Neuroseg *locseg = New_Local_Neuroseg();

  Set_Local_Neuroseg(
        locseg,  r + r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);

  double score = optimize(locseg);
  if (score >= m_seedMinScore) {
    m_locsegList.push_back(locseg);

    Local_Neuroseg *tailSeg = m_locsegList.back();
    Local_Neuroseg *headSeg = m_locsegList.front();

    while (tailSeg != NULL && headSeg != NULL) {
      if (tailSeg != NULL) {
        locseg = Next_Local_Neuroseg(tailSeg, NULL, 0.5);
        score = fit(locseg);
        if (score >= m_traceMinScore && !hitTraced(locseg)) {
          m_locsegList.push_back(locseg);
          tailSeg = locseg;
        } else {
          Kill_Local_Neuroseg(locseg);
          tailSeg = NULL;
        }
      }

      if (headSeg != NULL) {
        Flip_Local_Neuroseg(headSeg);
        locseg = Next_Local_Neuroseg(headSeg, NULL, 0.5);
        Flip_Local_Neuroseg(headSeg);
        score = fit(locseg);
        if (score >= m_traceMinScore && !hitTraced(locseg)) {
          Flip_Local_Neuroseg(locseg);
          m_locsegList.push_front(locseg);
          headSeg = locseg;
        } else {
          Kill_Local_Neuroseg(locseg);
          headSeg = NULL;
        }
      }
    }
  } else {
    Kill_Local_Neuroseg(locseg);
  }
}
