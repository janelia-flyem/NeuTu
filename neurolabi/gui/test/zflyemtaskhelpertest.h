#ifndef ZFLYEMTASKHELPERTEST_H
#define ZFLYEMTASKHELPERTEST_H

#include "ztestheader.h"
#include "flyem/zflyemtaskhelper.h"
#include "protocols/protocoltaskfactory.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmTaskHelper, Todo)
{
  ZFlyEmTaskHelper helper;

  ASSERT_EQ(neutu::TO_SPLIT, helper.getDefaultTodoAction());

  helper.setCurrentTaskType(ProtocolTaskFactory::TASK_BODY_CLEAVE);
  ASSERT_EQ(neutu::TO_SUPERVOXEL_SPLIT, helper.getDefaultTodoAction());

  helper.setCurrentTaskType(ProtocolTaskFactory::TASK_BODY_MERGE);
  ASSERT_EQ(neutu::TO_SPLIT, helper.getDefaultTodoAction());
}

#endif

#endif // ZFLYEMTASKHELPERTEST_H
