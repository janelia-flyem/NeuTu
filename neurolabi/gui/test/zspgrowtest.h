#ifndef ZSPGROWTEST_H
#define ZSPGROWTEST_H

#include "ztestheader.h"
#include "zspgrowparser.h"
#include "c_stack.h"
#include "tz_darray.h"

#ifdef _USE_GTEST_
TEST(ZSpGrowParser, Basic)
{
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  Stack *stack = C_Stack::make(GREY, 5, 5, 1);
  C_Stack::setZero(stack);
  C_Stack::setPixel(stack, 1, 1, 1, 0, 1);
  size_t seeds[] = {0, 1, 2};
  size_t targets[] = {2, 3, 4};
  sgw->conn = 8;

  Sp_Grow_Workspace_Enable_Eucdist_Buffer(sgw);
//  sgw->length = darray_malloc(C_Stack::voxelNumber(stack));
  Stack_Sp_Grow(stack, seeds, 1, targets, 1, sgw);

  ZSpGrowParser parser(sgw);

  darray_print2(sgw->length, C_Stack::width(stack), C_Stack::height(stack));

 Stack *distStack =  parser.createDistanceStack();
 C_Stack::printValue(distStack);
 C_Stack::kill(distStack);
}

#endif

#endif // ZSPGROWTEST_H
