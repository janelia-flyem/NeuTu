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

  ASSERT_FALSE(te.hit(16, 16, 28));
  ASSERT_FALSE(te._getHitItem().isValid());

  te.addItem(ZFlyEmToDoItem(16, 16, 28));
  ASSERT_TRUE(te.hit(16, 16, 16));
  ASSERT_TRUE(te._getHitItem().isValid());
  ASSERT_TRUE(te.hit(16, 16, 28));
  ASSERT_TRUE(te._getHitItem().isValid());
  ASSERT_EQ(ZIntPoint(16, 16, 28), te._getHitItem().getPosition());

  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_TRUE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te._getHitItem().isValid());

  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_TRUE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16));
  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_TRUE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());

  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_TRUE(te.isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_TRUE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 28));
  te.processHit(ZStackObject::ESelection::SELECT_TOGGLE);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_TRUE(te.isSelected());

  te.processHit(ZStackObject::ESelection::DESELECT);
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  ASSERT_TRUE(te.hit(16, 16, 16));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te.hit(16, 16, 28));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_TRUE(te.isSelected());

  te.deselect(true);
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 16).isSelected());
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_FALSE(te.isSelected());

  te.addItem(ZFlyEmToDoItem(16, 16, 33));
  ASSERT_TRUE(te.hit(16, 16, 28));
  te.processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());

  ASSERT_TRUE(te.hit(16, 16, 31));
  te.processHit(ZStackObject::ESelection::SELECT_SINGLE);
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 28).isSelected());
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 33).isSelected());

  auto errorProcessor = [](const std::string &str) {
    std::cout << str << std::endl;
  };
  int count = te.removeSelected(errorProcessor);
  ASSERT_EQ(1, count);

  ASSERT_FALSE(te._getBlockGrid()->getItem(16, 16, 33).isValid());
  ZFlyEmToDoItem item(16, 16, 33);
  item.setSelected(true);
  te.addItem(item);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 33).isSelected());

  item.setSelected(false);
  te.addItem(item);
  ASSERT_FALSE(te._getBlockGrid()->getExistingItem(16, 16, 33).isSelected());

  item.setSelected(true);
  te.addItem(item);
  ASSERT_TRUE(te._getBlockGrid()->getExistingItem(16, 16, 33).isSelected());

  item.setPosition(30, 40, 50);
  item.setSelected(true);
  te.addItem(item);
  count = te.removeSelected(errorProcessor);
  ASSERT_EQ(2, count);
}

#endif

#endif // FLYEMTODOENSEMBLETEST_H
