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
