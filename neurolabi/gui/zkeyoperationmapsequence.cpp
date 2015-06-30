#include "zkeyoperationmapsequence.h"
#include "zkeyoperationmap.h"

ZKeyOperationMapSequence::ZKeyOperationMapSequence()
{
}

ZStackOperator::EOperation ZKeyOperationMapSequence::getOperation(
    int key, Qt::KeyboardModifiers modifiers) const
{
  ZStackOperator::EOperation op = ZStackOperator::OP_NULL;
  for (QList<ZKeyOperationMap*>::const_iterator iter = m_mapList.begin();
       iter != m_mapList.end(); ++iter) {
    ZKeyOperationMap *keyMap = *iter;
    if (keyMap != NULL) {
      if (keyMap->hasKey(key, modifiers)) {
        op = keyMap->getOperation(key, modifiers);
        break;
      }
    }
  }

  return op;
}
