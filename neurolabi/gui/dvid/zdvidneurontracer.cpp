#include "zdvidneurontracer.h"

#include "zintcuboid.h"
#include "tz_math.h"

ZDvidNeuronTracer::ZDvidNeuronTracer()
{
  init();
}

void ZDvidNeuronTracer::init()
{
  m_resultTree = NULL;
}

void ZDvidNeuronTracer::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidReader.open(target);
}

ZSwcTree* ZDvidNeuronTracer::getResult() const
{
  return m_resultTree;
}

ZStack* ZDvidNeuronTracer::readStack(const ZIntCuboid &box)
{
  ZStack *stack = m_dvidReader.readGrayScale(box);

  return stack;
}

void ZDvidNeuronTracer::trace(double x, double y, double z, double r)
{
  ZIntCuboid box;

  Local_Neuroseg locseg;

  Set_Local_Neuroseg(
        &locseg,  r + r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);

  r *= 3.0;

  int width = iround(r + r);

  ZIntPoint firstCorner(iround(x - r), iround(y - r), iround(z - r));
  ZIntPoint lastConer = firstCorner + width;

  box.set(firstCorner, lastConer);

  ZStack *stack = readStack(box);

  ZIntPoint stackOffset = stack->getOffset();

  double pos[3];
  pos[0] = x - stackOffset.getX();
  pos[1] = y - stackOffset.getY();
  pos[2] = z - stackOffset.getZ();

  Set_Neuroseg_Position(&locseg, pos, NEUROSEG_CENTER);

  Locseg_Fit_Workspace *fws = New_Locseg_Fit_Workspace();
  fws->sws->fs.n = 2;
  fws->sws->fs.options[0] = STACK_FIT_DOT;
  fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;
  fws->pos_adjust = 1;

  Local_Neuroseg_Optimize_W(&locseg, stack->c_stack(), 1.0, 0, fws);

  Kill_Locseg_Fit_Workspace(fws);



  delete stack;
}
