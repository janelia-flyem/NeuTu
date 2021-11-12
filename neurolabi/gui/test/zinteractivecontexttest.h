#ifndef ZINTERACTIVECONTEXTTEST_H
#define ZINTERACTIVECONTEXTTEST_H

#include "ztestheader.h"
#include "zinteractivecontext.h"

#ifdef _USE_GTEST_

TEST(ZInteractiveContext, basic)
{
  ZInteractiveContext context;
  context.setTraceMode(ZInteractiveContext::TRACE_SINGLE);
  ASSERT_EQ(ZInteractiveContext::TRACE_SINGLE, context.traceMode());
  ASSERT_EQ(ZInteractiveContext::INTERACT_FREE, context.getUniqueMode());
  ASSERT_TRUE(context.fittingSegment());
  ASSERT_TRUE(context.isFreeMode());

  context.setMarkPunctaMode(ZInteractiveContext::MARK_PUNCTA);
  ASSERT_EQ(ZInteractiveContext::INTERACT_FREE, context.markingPuncta());
  ASSERT_EQ(ZInteractiveContext::MARK_PUNCTA, context.editPunctaMode());
  ASSERT_EQ(ZInteractiveContext::INTERACT_PUNCTA_MARK, context.getUniqueMode());

  context.setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);
  ASSERT_EQ(ZInteractiveContext::INTERACT_PUNCTA_MARK, context.getUniqueMode());
  ASSERT_EQ(ZInteractiveContext::MARK_PUNCTA, context.editPunctaMode());
  ASSERT_TRUE(context.markingPuncta());

  context.setMarkPunctaMode(ZInteractiveContext::MARK_PUNCTA_OFF);
  ASSERT_EQ(ZInteractiveContext::INTERACT_ADD_BOOKMARK, context.getUniqueMode());
  context.setBookmarkEditMode(ZInteractiveContext::BOOKMARK_EDIT_OFF);

  ASSERT_TRUE(context.isStackSliceView());
  ASSERT_TRUE(context.isObjectSliceView());
  context.setViewMode(ZInteractiveContext::ViewMode::VIEW_PROJECT);
  ASSERT_TRUE(context.isProjectView());
  ASSERT_FALSE(context.isStackSliceView());
  ASSERT_FALSE(context.isObjectSliceView());
  context.setViewMode(ZInteractiveContext::ViewMode::VIEW_OBJECT_PROJECT);
  ASSERT_TRUE(context.isStackSliceView());
  ASSERT_FALSE(context.isObjectSliceView());
  ASSERT_EQ(ZInteractiveContext::INTERACT_FREE, context.getUniqueMode());

  context.setExploreMode(ZInteractiveContext::ExploreMode::EXPLORE_MOVE_IMAGE);
  ASSERT_EQ(ZInteractiveContext::INTERACT_IMAGE_MOVE, context.getUniqueMode());
  context.setExploreMode(ZInteractiveContext::ExploreMode::EXPLORE_LOCAL);
  ASSERT_EQ(ZInteractiveContext::INTERACT_EXPLORE_LOCAL, context.getUniqueMode());

  context.setSwcEditMode(ZInteractiveContext::SwcEditMode::SWC_EDIT_EXTEND);
  ASSERT_EQ(ZInteractiveContext::INTERACT_EXPLORE_LOCAL, context.getUniqueMode());
  context.setExploreMode(ZInteractiveContext::ExploreMode::EXPLORE_OFF);
  ASSERT_EQ(ZInteractiveContext::EUniqueMode::INTERACT_SWC_EXTEND,
            context.getUniqueMode());

  context.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  ASSERT_EQ(ZInteractiveContext::EUniqueMode::INTERACT_SWC_EXTEND,
            context.getUniqueMode());
  context.turnOffEditMode();
  context.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  ASSERT_EQ(ZInteractiveContext::EUniqueMode::INTERACT_STROKE_DRAW,
            context.getUniqueMode());

  context.setRectEditMode(ZInteractiveContext::RECT_DRAW);
  ASSERT_EQ(ZInteractiveContext::EUniqueMode::INTERACT_STROKE_DRAW,
            context.getUniqueMode());
  context.turnOffEditMode();
  context.setRectEditMode(ZInteractiveContext::RECT_DRAW);
  ASSERT_EQ(ZInteractiveContext::INTERACT_RECT_DRAW, context.getUniqueMode());

  ASSERT_EQ(neutu::mvc::ERectTarget::PLANE_ROI, context.getRectTarget());
  context.setRectTarget(neutu::mvc::ERectTarget::CUBOID_ROI);
  ASSERT_EQ(neutu::mvc::ERectTarget::CUBOID_ROI, context.getRectTarget());

  context.setSynapseEditMode(ZInteractiveContext::SYNAPSE_MOVE);
  ASSERT_EQ(ZInteractiveContext::INTERACT_RECT_DRAW, context.getUniqueMode());
  context.turnOffEditMode();
  context.setSynapseEditMode(ZInteractiveContext::SYNAPSE_MOVE);
  ASSERT_EQ(ZInteractiveContext::INTERACT_MOVE_SYNAPSE, context.getUniqueMode());
  context.setSynapseEditMode(ZInteractiveContext::SYNAPSE_ADD_PRE);
  ASSERT_EQ(ZInteractiveContext::INTERACT_ADD_SYNAPSE, context.getUniqueMode());

  context.setTodoEditMode(ZInteractiveContext::TODO_ADD_ITEM);
  ASSERT_EQ(ZInteractiveContext::INTERACT_ADD_SYNAPSE, context.getUniqueMode());
  context.turnOffEditMode();
  context.setTodoEditMode(ZInteractiveContext::TODO_ADD_ITEM);
  ASSERT_EQ(ZInteractiveContext::INTERACT_ADD_TODO_ITEM, context.getUniqueMode());

  ASSERT_FALSE(context.isEditOff());
  ASSERT_FALSE(context.isContextMenuActivated());
  context.turnOffEditMode();
  ASSERT_TRUE(context.isEditOff());
  ASSERT_TRUE(context.isContextMenuActivated());
  context.blockContextMenu(true);
  ASSERT_FALSE(context.isContextMenuActivated());
}

#endif

#endif // ZINTERACTIVECONTEXTTEST_H
