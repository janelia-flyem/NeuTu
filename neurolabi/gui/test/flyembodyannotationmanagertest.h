#ifndef FLYEMBODYANNOTATIONMANAGERTEST_H
#define FLYEMBODYANNOTATIONMANAGERTEST_H

#include "ztestheader.h"

#include "zjsonobject.h"
#include "flyem/flyembodyannotationmanager.h"
#include "flyem/zflyembodyannotation.h"
#include "flyem/flyembodyannotationlocalio.h"

#ifdef _USE_GTEST_

TEST(FlyEmBodyAnnotationManager, Basic)
{
  FlyEmBodyAnnotationManager manager;

  ASSERT_TRUE(manager.getAnnotation(1).isEmpty());
  ZJsonObject obj;
  obj.setEntry("status", "Traced");
  manager.saveAnnotation(1, obj);

  obj.setEntry("status", "Anchor");
  manager.saveAnnotation(2, obj);

  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  obj = manager.getAnnotation(1);
  obj.setEntry("status", "Anchor");
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));

  manager.invalidateCache();
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  obj.setEntry("status", "Traced");
  manager.saveAnnotation(1, obj);
  obj.setEntry("status", "Anchor");
  manager.saveAnnotation(2, obj);

  ASSERT_TRUE(manager.isCached(1));
  ASSERT_TRUE(manager.isCached(2));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  manager.invalidateCache(2);
  ASSERT_TRUE(manager.isCached(1));
  ASSERT_FALSE(manager.isCached(2));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  std::shared_ptr<FlyEmBodyAnnotationLocalIO> io(new FlyEmBodyAnnotationLocalIO);
  manager.setIO(io);
  ASSERT_FALSE(manager.isCached(1));
  ASSERT_FALSE(manager.isCached(2));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  obj.setEntry("status", "Traced");
  manager.saveAnnotation(1, obj);
  obj.setEntry("status", "Anchor");
  manager.saveAnnotation(2, obj);
  ASSERT_TRUE(manager.isCached(1));
  ASSERT_TRUE(manager.isCached(2));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  manager.invalidateCache();
  ASSERT_FALSE(manager.isCached(1));
  ASSERT_FALSE(manager.isCached(2));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));
  ASSERT_TRUE(manager.isCached(1));
  ASSERT_TRUE(manager.isCached(2));

  manager.removeAnnotation(1);
  ASSERT_FALSE(manager.isCached(1));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));
  ASSERT_TRUE(manager.isCached(1));

  manager.removeAnnotation(2);
  ASSERT_FALSE(manager.isCached(2));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));
  ASSERT_TRUE(manager.isCached(2));

  obj.setEntry("status", "Traced");
  manager.saveAnnotation(1, obj);
  obj.setEntry("status", "Anchor");
  manager.saveAnnotation(2, obj);
  ASSERT_TRUE(manager.isCached(1));
  ASSERT_TRUE(manager.isCached(2));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Anchor", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(2)));

  io->setNonexistingException(true);
  obj.setEntry("status", "Test");
  io->writeBodyAnnotation(1, obj);
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Test", ZFlyEmBodyAnnotation::GetStatus(
              manager.getAnnotation(
                1, FlyEmBodyAnnotationManager::ECacheOption::SOURCE_FIRST)));
  io->deleteBodyAnnotation(1);
  ASSERT_EQ("Test", ZFlyEmBodyAnnotation::GetStatus(manager.getAnnotation(1)));
  ASSERT_EQ("Test", ZFlyEmBodyAnnotation::GetStatus(
              manager.getAnnotation(
                1, FlyEmBodyAnnotationManager::ECacheOption::SOURCE_FIRST)));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetStatus(
              manager.getAnnotation(
                1, FlyEmBodyAnnotationManager::ECacheOption::SOURCE_ONLY)));
}

#endif

#endif // FLYEMBODYANNOTATIONMANAGERTEST_H
