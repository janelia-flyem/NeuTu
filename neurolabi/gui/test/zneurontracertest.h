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

TEST(ZNeuronTracer, addTraceMask)
{
  ZNeuronTracer tracer;
  ZStack *stack = ZStackFactory::MakeZeroStack(5, 4, 3);

  ZStack *mask = ZStackFactory::MakeZeroStack(5, 4, 3);
  mask->setIntValue(3, 2, 1, 0, 1);


  tracer.bindSource(stack);
  tracer.addTraceMask(mask->c_stack());
  Stack *traceMask = tracer.getTraceMask();
//  C_Stack::printValue(traceMask);
  ASSERT_EQ(1, C_Stack::value(traceMask, 3, 2, 1, 0));

  ZStack *mask2 = ZStackFactory::MakeZeroStack(5, 4, 3);
  mask2->setIntValue(3, 2, 1, 0, 2);
  mask2->setIntValue(3, 2, 2, 0, 3);
  tracer.addTraceMask(mask2->c_stack());
  ASSERT_EQ(1, C_Stack::value(traceMask, 3, 2, 1, 0));
  ASSERT_EQ(3, C_Stack::value(traceMask, 3, 2, 2, 0));
  ASSERT_EQ(0, C_Stack::value(traceMask, 1, 2, 1, 0));

}

#endif

#endif // ZNEURONTRACERTEST_H
