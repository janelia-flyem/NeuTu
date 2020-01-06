#ifndef ZNEURONTRACERTEST_H
#define ZNEURONTRACERTEST_H

#include "ztestheader.h"
#include "zneurontracer.h"
#include "zstackfactory.h"
#include "c_stack.h"

#ifdef _USE_GTEST_

TEST(ZNeuronTracer, Init)
{
  ZStack *stack = ZStackFactory::MakeZeroStack(5, 4, 3);
  ZNeuronTracer tracer;
  tracer.bindSource(stack);

  ASSERT_EQ(nullptr, tracer.getTraceMask());

  ASSERT_EQ(stack->c_stack(), tracer.getIntensityData());

  tracer.initTraceMask(true);

  Trace_Workspace *tw = tracer.getTraceWorkspace();
  ASSERT_NE(nullptr, tw);
  ASSERT_TRUE(C_Stack::HasSameSize(tw->trace_mask, stack->c_stack()));
//  tracer.initTraceMask(false);

}

#endif

#endif // ZNEURONTRACERTEST_H
