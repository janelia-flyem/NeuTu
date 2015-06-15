#include "zkeyeventmapper.h"

#include <QSet>
#include <QKeyEvent>

#include "zstackoperator.h"

ZKeyEventMapper::ZKeyEventMapper()
{
}

ZStackOperator ZKeyEventMapper::getOperation(const QKeyEvent &event) const
{
  ZStackOperator op;
  op.setOperation(m_operationMap[event.key()]);

  return op;
}

void ZKeyEventMapper::merge(
    const ZKeyEventMapper &mapper, EKeyConflictResolve keyResolve,
    EValueConflictResolve valueResolve)
{
  QMap<int, ZStackOperator::EOperation> newMap = mapper.m_operationMap;

  for (QMap<int, ZStackOperator::EOperation>::iterator
       iter = m_operationMap.begin(); iter != m_operationMap.end(); ++iter) {
    ZStackOperator::EOperation op = mapper.m_operationMap[iter.key()];
    if (mapper.m_operationMap.contains(iter.key()) && (op != iter.value())) { //Value conflict
      switch (valueResolve) {
      case VALUE_KEEP_MASTER:
        newMap.remove(iter.key());
        break;
      case VALUE_KEEP_GUEST:
        break;
      }
    }
  }

  if (keyResolve != KEY_KEEP_BOTH) {
    QSet<ZStackOperator::EOperation> guestValueSet;
    guestValueSet.fromList(mapper.m_operationMap.values());

    for (QSet<ZStackOperator::EOperation>::const_iterator
         iter = guestValueSet.begin(); iter != guestValueSet.end(); ++iter) {
      ZStackOperator::EOperation op = *iter;
      QList<int> masterKeys = m_operationMap.keys(op);
      if (!masterKeys.empty()) {
        switch (keyResolve) {
        case KEY_KEEP_GUEST:
        {
          foreach (int key, masterKeys) {
            m_operationMap.remove(key);
          }
          QList<int> guestKeys = mapper.m_operationMap.keys(op);
          foreach (int key, guestKeys) {
            newMap[key] = op;
          }
        }
          break;
        default:
          break;
        }
      }
    }
  }

  m_operationMap.unite(newMap);
}

////////////////////////////////////

ZStackOperator ZKeyPressEventMapper::getOperation(const QKeyEvent &event) const
{
  ZStackOperator op;
  op.setOperation(m_operationMap[event.key()]);

  return op;
}
