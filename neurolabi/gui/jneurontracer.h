#ifndef JNEURONTRACER_H
#define JNEURONTRACER_H

#include "zswcpath.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "neutube_def.h"
#include "zstackgraph.h"
#include "tz_locseg_chain.h"
#include "zprogressable.h"
#include "zintpoint.h"

class ZStack;
class ZSwcTree;

class JNeuronTracer
{
public:
  JNeuronTracer();

  /*!
   * \brief Auto trace
   *
   * It will also create workspaces automatically if necessary.
   */
  ZSwcTree* trace(const Stack *stack, bool doResampleAfterTracing = true);
  Stack* makeMask(const Stack *stack);

private:
  //gaussian filter in 2d, modified from Vaa3D plugin gaussianfilter.
  static void gaussianFilter2D(int nx, int ny, unsigned int Wx, unsigned int Wy,
                               float *imgIn, float *img, float sigma);

  static void createMask2D(int nx, int ny, float *I, float *bw);

private:
//  NeuTube::EImageBackground m_background;
  int m_splitNumber;

};

#endif // JNEURONTRACER_H
