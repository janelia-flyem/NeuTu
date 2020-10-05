#ifndef FLYEMTODOENSEMBLETEST_H
#define FLYEMTODOENSEMBLETEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "flyem/flyemtodoensemble.h"
#include "flyem/flyemtodomocksource.h"
#include "flyem/flyemtodoblockgrid.h"

TEST(FlyEmTodoEnsemble, Basic)
{
  FlyEmTodoEnsemble te;

  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  te._setSource(source);

  ASSERT_FALSE(te.getItem({160, 160, 160}).isValid());
  source->saveItem(ZFlyEmToDoItem({160, 160, 160}));
  ASSERT_FALSE(te.getItem({160, 160, 160}).isValid());
  te.updateItem({160, 160, 160});
  ASSERT_TRUE(te.getItem({160, 160, 160}).isValid());

  ASSERT_FALSE(te.hit(16, 16, 28, 0));
  ASSERT_FALSE(te._getHitItem().isValid());

  ZFlyEmToDoItem item = ZFlyEmToDoItem(16, 16, 28);
  item.setDefaultHit();

  te.addItem(item);
  ASSERT_TRUE(te.hit(16, 16, 16, 0));
  ASSERT_TRUE(te._getHitItem().isValid());
  ASSERT_TRUE(te.hit(16, 16, 28, 0));
  ASSERT_TRUE(te._getHitItem().isValid());
  ASSERT_EQ(ZIntPoint(16, 16, 28), te._getHitItem().getPosition());

  ASSERT_FALSE(te.getItem({16, 16, 16}).hasPartner({26, 36, 46}));
  te.updatePartner({16, 16, 16});
  ASSERT_TRUE(te.getItem({16, 16, 16}).hasPartner({26, 36, 46}));

  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_FALSE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te._getHitItem().isValid());

  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_FALSE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16, 0));
  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_FALSE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());

  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_FALSE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16, 0));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16, 0));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 28, 0));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  te.processHit(ZStackObject::ESelection::DESELECT);
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16, 0));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te.hit(16, 16, 28, 0));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  te.deselect(true);
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  item.setPosition(16, 16, 33);
  te.addItem(item);
  ASSERT_TRUE(te.hit(16, 16, 28, 0));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 31, 0));
  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 28).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 33).isSelected());

  auto errorProcessor = [](const std::string &str) {
    std::cout << str << std::endl;
  };
  int count = te.removeSelected(errorProcessor);
  ASSERT_EQ(1, count);

  ASSERT_FALSE(te._getBlockGrid()->getItem(16, 16, 33).isValid());
  item.setPosition(16, 16, 33);
  item.setSelected(true);
  te.addItem(item);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 33).isSelected());

  item.setSelected(false);
  te.addItem(item);
  ASSERT_FALSE(te._getBlockGrid()->getCachedItem(16, 16, 33).isSelected());

  item.setSelected(true);
  te.addItem(item);
  ASSERT_TRUE(te._getBlockGrid()->getCachedItem(16, 16, 33).isSelected());

  item.setPosition(30, 40, 50);
  item.setSelected(true);
  te.addItem(item);
  count = te.removeSelected(errorProcessor);
  ASSERT_EQ(2, count);

  te.addItem(ZFlyEmToDoItem(30, 40, 50));
  te.moveItem({30, 40, 50}, {300, 400, 500});
  ASSERT_TRUE(te.getItem({300, 400, 500}).isValid());
  ASSERT_FALSE(te.getItem({30, 40, 50}).isValid());
}

#endif

#endif // FLYEMTODOENSEMBLETEST_H
