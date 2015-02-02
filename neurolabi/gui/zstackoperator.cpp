#include "zstackoperator.h"
#include "zmouseeventrecorder.h"
#include "zstroke2d.h"
#include "zswctree.h"

ZStackOperator::ZStackOperator() :
  m_op(OP_NULL),
  m_hitObject(NULL),
  m_punctaIndex(-1), m_togglingStrokeLabel(false), m_mouseEventRecorder(NULL)
{
}

bool ZStackOperator::isNull() const
{
  return getOperation() == OP_NULL;
}

ZPoint ZStackOperator::getMouseOffset(NeuTube::ECoordinateSystem cs) const
{
  ZPoint offset(0, 0, 0);

  if (m_mouseEventRecorder != NULL) {
    offset = m_mouseEventRecorder->getPositionOffset(cs);
  }

  return offset;
}

template<>
Swc_Tree_Node* ZStackOperator::getHitObject<Swc_Tree_Node>() const
{
  Swc_Tree_Node *tn = NULL;
  ZSwcTree *tree = getHitObject<ZSwcTree>();
  if (tree != NULL) {
    tn = tree->getHitNode();
  }

  return tn;
}
