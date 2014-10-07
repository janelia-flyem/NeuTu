#ifndef ZSTACKDOCHITTEST_H
#define ZSTACKDOCHITTEST_H

#include "swctreenode.h"

class ZStroke2d;
class ZStackDoc;
class ZPoint;

class ZStackDocHitTest
{
public:
  ZStackDocHitTest();

  bool hitTest(const ZStackDoc *doc, double x, double y, double z);
  bool hitTest(const ZStackDoc *doc, const ZPoint &pt);
  bool hitTest(const ZStackDoc *doc, double x, double y);

  Swc_Tree_Node* getHitSwcNode() const {
    return m_hitSwcNode;
  }

  ZStroke2d* getHitStroke2d() const {
    return m_hitStroke;
  }

private:
  Swc_Tree_Node *m_hitSwcNode;
  ZStroke2d *m_hitStroke;
};

#endif // ZSTACKDOCHITTEST_H
