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
#include "zresolution.h"

class ZStack;
class ZSwcTree;

class JNeuronTracer
{
public:
  JNeuronTracer();
  ~JNeuronTracer();

  /*!
   * \brief Auto trace
   *
   * It will also create workspaces automatically if necessary.
   */
  ZSwcTree* trace(const Stack *stack, bool doResampleAfterTracing = true);
  Stack* makeMask(const Stack *stack);
  void setResolution(const ZResolution &resolution);

  void setMask(Stack *mask) {
    m_maskData = mask;
  }

private:
  //gaussian filter in 2d, modified from Vaa3D plugin gaussianfilter.
  static void gaussianFilter2D(int nx, int ny, unsigned int Wx, unsigned int Wy,
                               float *imgIn, float *img, float sigma);

  static void createMask2D(int nx, int ny, float *I, float *bw);
  static void levelsetSmoothing(int nx, int ny, float *bw, float *edge,int iter);

private:
//  NeuTube::EImageBackground m_background;
  int m_splitNumber;
  ZResolution m_resolution;
  Stack *m_maskData;

};

#endif // JNEURONTRACER_H
