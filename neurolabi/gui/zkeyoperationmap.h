#ifndef ZKEYOPERATIONMAP_H
#define ZKEYOPERATIONMAP_H

#include <QMap>
#include <QList>
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
  ZStackOperator::EOperation getOperation(
      int key, Qt::KeyboardModifiers modifiers) const;
  /*!
   * \brief Is the key mapped or not.
   *
   * \return true iff \a key is mapped, even if the mapped operation is null.
   */
  bool hasKey(int key, Qt::KeyboardModifiers modifiers) const;


  enum EValueConflictResolve { //Same key to different value
    VALUE_KEEP_MASTER, VALUE_KEEP_GUEST,
  };

  enum EKeyConflictResolve { //Different key to same value
    KEY_KEEP_BOTH, KEY_KEEP_MASTER, KEY_KEEP_GUEST
  };

  static void merge(QMap<int, ZStackOperator::EOperation> &master,
                    const QMap<int, ZStackOperator::EOperation> &guest,
                    EKeyConflictResolve keyResolve,
                    EValueConflictResolve valueResolve);

  void merge(const ZKeyOperationMap &mapper, EKeyConflictResolve keyResolve,
             EValueConflictResolve valueResolve);

private:
  QMap<int, ZStackOperator::EOperation> m_plainMap;
  QMap<int, ZStackOperator::EOperation> m_controlMap;
  QMap<int, ZStackOperator::EOperation> m_altMap;
  QMap<int, ZStackOperator::EOperation> m_shiftMap;
};

#endif // ZKEYOPERATIONMAP_H
