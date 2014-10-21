#ifndef ZSTACKDOCHITTEST_H
#define ZSTACKDOCHITTEST_H

#include "swctreenode.h"

class ZStroke2d;
class ZStackDoc;
class ZPoint;
class ZObject3d;
class ZStackObject;

class ZStackDocHitTest
{
public:
  ZStackDocHitTest();

  bool hitTest(ZStackDoc *doc, double x, double y, double z);
  bool hitTest(ZStackDoc *doc, const ZPoint &pt);
  bool hitTest(ZStackDoc *doc, double x, double y);

  Swc_Tree_Node* getHitSwcNode() const;

  ZStroke2d* getHitStroke2d() const;

  ZObject3d* getObj3d() const;

private:
  ZStackObject *m_hitObject;
//  Swc_Tree_Node *m_hitSwcNode;
//  ZStroke2d *m_hitStroke;
//  ZObject3d *m_hitObj3d;
};

#endif // ZSTACKDOCHITTEST_H
