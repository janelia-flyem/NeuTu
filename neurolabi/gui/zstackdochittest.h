#ifndef ZSTACKDOCHITTEST_H
#define ZSTACKDOCHITTEST_H

#include "swctreenode.h"

class ZStroke2d;
class ZStackDoc;
class ZPoint;
class ZObject3d;
class ZStackObject;
class ZPunctum;

class ZStackDocHitTest
{
public:
  ZStackDocHitTest();

  bool hitTest(ZStackDoc *doc, double x, double y, double z);
  bool hitTest(ZStackDoc *doc, const ZPoint &pt);
  bool hitTest(ZStackDoc *doc, double x, double y, NeuTube::EAxis axis);

//  Swc_Tree_Node* getHitSwcNode() const;
//  ZStroke2d* getHitStroke2d() const;
//  ZObject3d* getObj3d() const;
//  ZPunctum* getPunctum() const;

  template <typename T>
  T* getHitObject() const;


private:
  ZStackObject *m_hitObject;
 // Swc_Tree_Node *m_hitSwcNode;
//  ZStroke2d *m_hitStroke;
//  ZObject3d *m_hitObj3d;
};

template<>
Swc_Tree_Node* ZStackDocHitTest::getHitObject<Swc_Tree_Node>() const;

template <typename T>
T* ZStackDocHitTest::getHitObject() const
{
  return dynamic_cast<T*>(m_hitObject);
}

#endif // ZSTACKDOCHITTEST_H
