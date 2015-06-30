#include "zkeyoperationmap.h"

ZKeyOperationMap::ZKeyOperationMap()
{
}

const QMap<int, ZStackOperator::EOperation>* ZKeyOperationMap::getMap(
    Qt::KeyboardModifiers modifiers) const
{
  switch (modifiers) {
  case Qt::NoModifier:
    return getPlainMap();
  case Qt::AltModifier:
    return getAltMap();
  case Qt::ControlModifier:
    return getControlMap();
  case Qt::ShiftModifier:
    return getShiftMap();
  default:
    return NULL;
  }

  return NULL;
}


ZStackOperator::EOperation ZKeyOperationMap::getOperation(
    int key, Qt::KeyboardModifiers modifiers) const
{
  const QMap<int, ZStackOperator::EOperation> *map = getMap(modifiers);
  if (map != NULL) {
    if (map->contains(key)) {
      return map->value(key);
    }
  }

  return ZStackOperator::OP_NULL;
}

bool ZKeyOperationMap::hasKey(int key, Qt::KeyboardModifiers modifiers) const
{
  if (getMap(modifiers) != NULL) {
    return getMap(modifiers)->contains(key);
  }

  return false;
}

void ZKeyOperationMap::merge(QMap<int, ZStackOperator::EOperation> &master,
                             const QMap<int, ZStackOperator::EOperation> &guest,
                             EKeyConflictResolve keyResolve,
                             EValueConflictResolve valueResolve)
{
  QMap<int, ZStackOperator::EOperation> newMap = guest;

  for (QMap<int, ZStackOperator::EOperation>::iterator
       iter = master.begin(); iter != master.end(); ++iter) {
    ZStackOperator::EOperation op = guest[iter.key()];
    //Value conflict: same key, but different value
    if (guest.contains(iter.key()) && (op != iter.value())) {
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
    guestValueSet.fromList(guest.values());

    for (QSet<ZStackOperator::EOperation>::const_iterator
         iter = guestValueSet.begin(); iter != guestValueSet.end(); ++iter) {
      ZStackOperator::EOperation op = *iter;
      QList<int> masterKeys = master.keys(op);
      QList<int> guestKeys = guest.keys(op);
      //Keys of same values
      if (!masterKeys.empty() && !guestKeys.empty()) {
        switch (keyResolve) {
        case KEY_KEEP_GUEST:
        {
          //Remove master keys
          foreach (int key, masterKeys) {
            master.remove(key);
          }
          //Record guest keys
          /*
          foreach (int key, guestKeys) {
            newMap[key] = op;
          }
          */
        }
          break;
        case KEY_KEEP_MASTER:
          //Remove guest keys there are master keys of the same value
          foreach (int key, guestKeys) {
            newMap.remove(key);
          }
          break;
        default:
          break;
        }
      }
    }
  }

  master.unite(newMap);
}

void ZKeyOperationMap::merge(
    const ZKeyOperationMap &mapper, EKeyConflictResolve keyResolve,
    EValueConflictResolve valueResolve)
{
  merge(m_plainMap, mapper.m_plainMap, keyResolve, valueResolve);
  merge(m_controlMap, mapper.m_controlMap, keyResolve, valueResolve);
  merge(m_altMap, mapper.m_altMap, keyResolve, valueResolve);
  merge(m_shiftMap, mapper.m_shiftMap, keyResolve, valueResolve);
}
