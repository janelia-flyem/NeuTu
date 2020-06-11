#include "zstackdochittest.h"
#include "mvc/zstackdoc.h"
#include "zstroke2d.h"
#include "geometry/zpoint.h"
#include "zobject3d.h"
#include "zswctree.h"

ZStackDocHitTest::ZStackDocHitTest() : m_hitObject(NULL)
{
  m_sliceAxis = neutu::EAxis::Z;
}

neutu::EAxis ZStackDocHitTest::getSliceAxis() const
{
  return m_sliceAxis;
}

void ZStackDocHitTest::setSliceAxis(neutu::EAxis axis)
{
  m_sliceAxis = axis;
}

bool ZStackDocHitTest::hitTest(ZStackDoc *doc, double x, double y)
{
  m_hitObject = doc->hitTest(x, y, getSliceAxis());
  return m_hitObject != NULL;


  /*
  m_hitStroke = NULL;
  m_hitSwcNode = NULL;

  const QList<ZStroke2d*> strokeList = doc->getStrokeList();
  foreach (const ZStroke2d *stroke, strokeList) {
    if (stroke->hitTest(x, y)) {
      m_hitStroke = const_cast<ZStroke2d*>(stroke);
      break;
    }
  }

  if (m_hitStroke == NULL) {
    m_hitSwcNode = doc->swcHitTest(x, y);
  }


  return m_hitStroke != NULL;
  */
}

bool ZStackDocHitTest::hitTest(
    ZStackDoc *doc, const ZPoint &pt, const ZIntPoint &widgetPosition)
{
  m_hitObject = doc->hitTest(pt.roundToIntPoint(), widgetPosition, getSliceAxis());
  return m_hitObject != NULL;

//  return hitTest(doc, pt.x(), pt.y(), pt.z());
}

bool ZStackDocHitTest::hitTest(ZStackDoc *doc, double x, double y, double z)
{
  m_hitObject = doc->hitTest(x, y, z);
  return m_hitObject != NULL;
  /*
  m_hitStroke = NULL;
  m_hitSwcNode = NULL;

  const QList<ZStroke2d*> strokeList = doc->getStrokeList();
  foreach (const ZStroke2d *stroke, strokeList) {
    if (stroke->hitTest(x, y, z)) {
      m_hitStroke = const_cast<ZStroke2d*>(stroke);
      break;
    }
  }

  if (m_hitStroke == NULL) {
    m_hitSwcNode = doc->swcHitTest(x, y, z);
  }

  return m_hitStroke != NULL;
  */
}

//Swc_Tree_Node* ZStackDocHitTest::getHitSwcNode() const
//{
//  Swc_Tree_Node *tn = NULL;
//  ZSwcTree *tree = dynamic_cast<ZSwcTree*>(m_hitObject);
//  if (tree != NULL) {
//    tn = tree->getHitNode();
//  }

//  return tn;
//}

//ZStroke2d* ZStackDocHitTest::getHitStroke2d() const
//{
//  return dynamic_cast<ZStroke2d*>(m_hitObject);
//}

//ZObject3d* ZStackDocHitTest::getObj3d() const
//{
//  return dynamic_cast<ZObject3d*>(m_hitObject);
//}

//ZPunctum* ZStackDocHitTest::getPunctum() const
//{
//  return dynamic_cast<ZPunctum*>(m_hitObject);
//}

template<>
Swc_Tree_Node* ZStackDocHitTest::getHitObject<Swc_Tree_Node>() const
{
  Swc_Tree_Node *tn = NULL;
  ZSwcTree *tree = getHitObject<ZSwcTree>();
  if (tree != NULL) {
    tn = tree->getHitNode();
  }

  return tn;
  //return m_hitSwcNode;
}
