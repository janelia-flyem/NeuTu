#ifndef ZKEYOPERATIONMAPSEQUENCE_H
#define ZKEYOPERATIONMAPSEQUENCE_H

#include <QList>
#include "zstackoperator.h"
#include "neutube_def.h"

class ZKeyOperationMap;

class ZKeyOperationMapSequence
{
public:
  ZKeyOperationMapSequence();

  /*!
   * \brief Get mapped operation
   *
   * \return The first mapped operation in the sequence.
   */
  ZStackOperator::EOperation getOperation(
      int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier) const;

private:
  QList<ZKeyOperationMap*> m_mapList;
};

#endif // ZKEYOPERATIONMAPSEQUENCE_H
