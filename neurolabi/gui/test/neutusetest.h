#ifndef NEUTUSETEST_H
#define NEUTUSETEST_H

#include "ztestheader.h"

#include "dvid/zdvidtarget.h"
#include "neutuse/taskfactory.h"
#include "neutuse/task.h"

#ifdef _USE_GTEST_

TEST(neutuse, TaskFactory)
{
  neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
        "test", ZDvidTarget(), 1, true);

  ASSERT_EQ("dvid", task.getType());
  ASSERT_EQ("test", task.getName());
  ASSERT_EQ(int(0), task.getPriority());

  task = neutuse::TaskFactory::MakeDvidSkeletonizeTask(ZDvidTarget(), 1, true);
  ASSERT_EQ("dvid", task.getType());
  ASSERT_EQ("skeletonize", task.getName());
  ASSERT_EQ(5, task.getPriority());
}

#endif

#endif // NEUTUSETEST_H
