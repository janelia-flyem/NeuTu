#ifndef ZKEYOPERATIONMAP_H
#define ZKEYOPERATIONMAP_H

#include <QMap>
#include <QKeyEvent>
#include "zstackoperator.h"

class ZKeyOperationMap
{
public:
  ZKeyOperationMap();

  inline QMap<int, ZStackOperator::EOperation>* getPlainMap() {
    return &m_plainMap;
  }

  inline QMap<int, ZStackOperator::EOperation>* getControlMap() {
    return &m_controlMap;
  }

  inline QMap<int, ZStackOperator::EOperation>* getAltMap() {
    return &m_altMap;
  }

  inline QMap<int, ZStackOperator::EOperation>* getShiftMap() {
    return &m_shiftMap;
  }

  inline const QMap<int, ZStackOperator::EOperation>* getPlainMap() const {
    return &m_plainMap;
  }

  inline const QMap<int, ZStackOperator::EOperation>* getControlMap() const {
    return &m_controlMap;
  }

  inline const QMap<int, ZStackOperator::EOperation>* getAltMap() const {
    return &m_altMap;
  }

  inline const QMap<int, ZStackOperator::EOperation>* getShiftMap() const {
    return &m_shiftMap;
  }

  const QMap<int, ZStackOperator::EOperation>* getMap(
      Qt::KeyboardModifiers modifiers) const;
  ZStackOperator::EOperation getOperation(int key, Qt::KeyboardModifiers modifiers) const;

private:
  QMap<int, ZStackOperator::EOperation> m_plainMap;
  QMap<int, ZStackOperator::EOperation> m_controlMap;
  QMap<int, ZStackOperator::EOperation> m_altMap;
  QMap<int, ZStackOperator::EOperation> m_shiftMap;
};

#endif // ZKEYOPERATIONMAP_H
