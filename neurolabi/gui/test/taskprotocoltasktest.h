#ifndef TASKPROTOCOLTASKTEST_H
#define TASKPROTOCOLTASKTEST_H

#include "ztestheader.h"

#include "protocols/taskprotocolmocktask.h"

#ifdef _USE_GTEST_

TEST(MockTaskProtocolTask, Tag)
{
  TaskProtocolTaskMock task;
  task.addTag("test");
  ASSERT_TRUE(task.hasTag("test"));
  task.removeTag("test");
  ASSERT_FALSE(task.hasTag("test"));
  task.toggleTag("test", true);
  ASSERT_TRUE(task.hasTag("test"));
  task.toggleTag("test", false);
  ASSERT_FALSE(task.hasTag("test"));

  task.addTag("test1");
  task.toggleTag("test1", true);
  task.toggleTag("test2", true);
  task.removeTag("test3");
  QStringList tags = task.getTags();
  ASSERT_EQ(2, tags.size());
  task.clearTags();
  ASSERT_FALSE(task.hasTag("test1"));
  tags = task.getTags();
  ASSERT_TRUE(tags.isEmpty());
}

#endif

#endif // TASKPROTOCOLTASKTEST_H
