#ifndef ZMOUSECURSORGLYPHTEST_H
#define ZMOUSECURSORGLYPHTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "mvc/zmousecursorglyph.h"
#include "zstackball.h"
#include "zswctree.h"
#include "zstroke2d.h"
#include "zweightedpoint.h"

namespace {
void set_single_node_mode(ZSwcTree *tree)
{
  if (tree) {
    Swc_Tree_Node *vtn = tree->root();
    Swc_Tree_Node *tn = tree->firstRegularRoot();
    if (SwcTreeNode::hasChild(tn)) {
      SwcTreeNode::setRadius(tn, vtn->node.d);
      Swc_Tree_Node *child = tn->first_child;
      while (child) {
        Swc_Tree_Node *nextChild = child->next_sibling;
        SwcTreeNode::killSubtree(child);
        child = nextChild;
      }
      tree->deprecate(ZSwcTree::ALL_COMPONENT);
    }
  }
}

void set_double_node_mode(
    ZSwcTree *tree, const ZPoint &startCenter, double endRadius)
{
  if (tree) {
    Swc_Tree_Node *tn = tree->firstRegularRoot();
    SwcTreeNode::setCenter(tn, startCenter);
    if (!SwcTreeNode::hasChild(tn)) {
      SwcTreeNode::setRadius(tree->root(), SwcTreeNode::radius(tn));
      SwcTreeNode::setRadius(tn, 0);
      tn = SwcTreeNode::MakeChild(tn);
    } else {
      tn = SwcTreeNode::firstChild(tn);
    }
    SwcTreeNode::setCenter(tn, startCenter);
    SwcTreeNode::setRadius(tn, endRadius);
    tree->deprecate(ZSwcTree::ALL_COMPONENT);
  }
}
}

TEST(ZMouseCursorGlyph, Basic)
{
  ZMouseCursorGlyph mcg;

  ASSERT_EQ(5, mcg.getGlyphList().size());
  ASSERT_FALSE(mcg.isActivated());
  ASSERT_FALSE(mcg.isActivated(ZMouseCursorGlyph::ROLE_BOOKMARK));

  mcg.activate(ZMouseCursorGlyph::ROLE_BOOKMARK);
  ASSERT_TRUE(mcg.isActivated());
  ASSERT_TRUE(mcg.isActivated(ZMouseCursorGlyph::ROLE_BOOKMARK));
  ASSERT_FALSE(mcg.isActivated(ZMouseCursorGlyph::ROLE_SWC));
  ZStackObject *deactivated = nullptr;
  ZStackObject *activated = nullptr;
  mcg.processActiveChange([&](ZStackObject *dg, ZStackObject *ag) {
    deactivated = dg;
    activated = ag;
  });
  ASSERT_EQ(nullptr, deactivated);
  ASSERT_EQ(mcg._getGlyph(ZMouseCursorGlyph::ROLE_BOOKMARK), activated);

  mcg.deactivate();
  mcg.processActiveChange([&](ZStackObject *dg, ZStackObject *ag) {
    deactivated = dg;
    activated = ag;
  });
  ASSERT_EQ(nullptr, activated);
  ASSERT_EQ(mcg._getGlyph(ZMouseCursorGlyph::ROLE_BOOKMARK), deactivated);

  ZStackBall *ball = mcg._getGlyph<ZStackBall>(ZMouseCursorGlyph::ROLE_BOOKMARK);
  ASSERT_TRUE(ball);
  ASSERT_NE(123, ball->getLabel());

  mcg.setPrepareFunc(ZMouseCursorGlyph::ROLE_BOOKMARK, [](ZStackObject *obj) {
    obj->setLabel(123);
  });
  mcg.activate(ZMouseCursorGlyph::ROLE_BOOKMARK);

  ASSERT_EQ(123, ball->getLabel());

  mcg.setPrepareFunc(ZMouseCursorGlyph::ROLE_BOOKMARK, [](ZStackObject *obj) {
    ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
    ball->setRadius(10.0);
    ball->setCenter(1, 2, 3);
  });

  mcg.activate(ZMouseCursorGlyph::ROLE_BOOKMARK);
  ASSERT_EQ(10.0, mcg.getActiveGlyphSize());
  ASSERT_EQ(ZPoint(1, 2, 3), mcg.getActiveGlyphPosition());
  ZWeightedPoint wpt = mcg.getActiveGlyphGeometry();
  ASSERT_EQ(ZPoint(1, 2, 3), wpt);
  ASSERT_EQ(10.0, wpt.weight());

  mcg.setActiveGlyphPosition(ZPoint(4, 5, 6), [](ZStackObject *obj) {
    obj->setLabel(20);
  });
  ASSERT_EQ(ZPoint(4, 5, 6), mcg.getActiveGlyphPosition());
  ASSERT_EQ(20, ball->getLabel());

  mcg.setActiveGlyphSize(11.0, [](ZStackObject *obj) {
    obj->setLabel(21);
  });
  ASSERT_EQ(ZPoint(4, 5, 6), mcg.getActiveGlyphPosition());
  ASSERT_EQ(11.0, mcg.getActiveGlyphSize());
  ASSERT_EQ(21, ball->getLabel());

  mcg.addActiveGlyphSize(5.0, [](ZStackObject *obj) {
    obj->setLabel(22);
  });
  ASSERT_EQ(16.0, mcg.getActiveGlyphSize());
  ASSERT_EQ(22, ball->getLabel());

  mcg.activate(ZMouseCursorGlyph::ROLE_STROKE);
  mcg.processActiveChange([&](ZStackObject *dg, ZStackObject *ag) {
    deactivated = dg;
    activated = ag;
  });

  ASSERT_EQ(ZStackObject::EType::STROKE, activated->getType());
  ASSERT_EQ(ZStackObject::EType::STACK_BALL, deactivated->getType());
  ASSERT_TRUE(mcg.isActivated(ZMouseCursorGlyph::ROLE_STROKE));

  mcg.setActiveGlyphPosition(
        ZPoint(4, 5, 6), std::function<void(ZStackObject*)>());
  mcg.appendActiveGlyphPosition(ZPoint(10, 11, 12), [](ZStackObject *obj) {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
    stroke->print();
  });

  ZStroke2d *stroke = mcg._getGlyph<ZStroke2d>(ZMouseCursorGlyph::ROLE_STROKE);
  ASSERT_EQ(2, stroke->getPointNumber());

  mcg.setActiveGlyphPosition(ZPoint(13, 14, 15), [](ZStackObject *obj) {
    obj->setLabel(2);
  });
  ASSERT_EQ(2, stroke->getLabel());
  ASSERT_EQ(2, stroke->getPointNumber());
  ASSERT_EQ(ZPoint(13, 14, 15), stroke->getLastPoint());

  mcg.setActiveGlyphSize(30.0, [](ZStackObject *obj) {
    obj->setLabel(3);
  });
  stroke->print();
  ASSERT_EQ(3, stroke->getLabel());
  ASSERT_EQ(30.0, mcg.getActiveGlyphSize());
  wpt = mcg.getActiveGlyphGeometry();
  ASSERT_EQ(ZPoint(13, 14, 15), wpt);
  ASSERT_EQ(15.0, wpt.weight());

  int mode = 1;
  mcg.setPrepareFunc(ZMouseCursorGlyph::ROLE_SWC, [&](ZStackObject *obj) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
    if (tree) {
      if (mode == 1) {
        set_single_node_mode(tree);
      } else if (mode == 2) {
        set_double_node_mode(tree, ZPoint(3, 4, 5), 9.0);
      }
    }
  });
  mcg.activate(ZMouseCursorGlyph::ROLE_SWC);
  mcg.setActiveGlyphPosition(ZPoint(1, 2, 3));
  mcg.setActiveGlyphSize(8.0);

  wpt = mcg.getActiveGlyphGeometry();
  ASSERT_EQ(ZPoint(1, 2, 3), wpt);
  ASSERT_EQ(8.0, wpt.weight());

  mode = 2;
  mcg.activate(ZMouseCursorGlyph::ROLE_SWC);
  ZSwcTree *tree = mcg._getGlyph<ZSwcTree>(ZMouseCursorGlyph::ROLE_SWC);
  tree->print();
  wpt = mcg.getActiveGlyphGeometry();
  ASSERT_EQ(ZPoint(3, 4, 5), wpt);
  ASSERT_EQ(9.0, wpt.weight());

  mode = 1;
  mcg.activate(ZMouseCursorGlyph::ROLE_SWC);
  ASSERT_EQ(8.0, mcg.getActiveGlyphSize());

  mcg.updateActiveGlyph([](ZStackObject *obj) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
    set_double_node_mode(tree, ZPoint(7, 8, 9), 10.0);
  });
  ASSERT_EQ(10.0, mcg.getActiveGlyphSize());
}

#endif


#endif // ZMOUSECURSORGLYPHTEST_H
