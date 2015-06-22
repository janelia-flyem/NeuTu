#ifndef ZKEYEVENTMAPPER_H
#define ZKEYEVENTMAPPER_H

#include <QMap>
#include "tz_utilities.h"
#include "zsharedpointer.h"
#include "zstackoperator.h"

//class ZInteractiveContext;
//class ZStackDoc;
//class ZStackOperator;
class QKeyEvent;

class ZKeyEventMapper
{
public:
  ZKeyEventMapper();

  enum EValueConflictResolve { //Same key to different value
    VALUE_KEEP_MASTER, VALUE_KEEP_GUEST,
  };

  enum EKeyConflictResolve { //Different key to same value
    KEY_KEEP_BOTH, KEY_KEEP_MASTER, KEY_KEEP_GUEST
  };

  /*
  ZKeyEventMapper(ZInteractiveContext *context = NULL,
                  ZStackDoc *doc = NULL);
                  */

  virtual ZStackOperator getOperation(const QKeyEvent &event) const;
  void setOperation(int key, ZStackOperator::EOperation op);

  ZStackOperator initOperation() const;

  void merge(const ZKeyEventMapper &mapper, EKeyConflictResolve keyResolve,
             EValueConflictResolve valueResolve);
/*
  inline void setContext(ZInteractiveContext *context) {
    m_context = context;
  }

  inline void setDocument(ZSharedPointer<ZStackDoc> doc) {
    m_doc = doc;
  }
*/
protected:
  QMap<int, ZStackOperator::EOperation> m_operationMap;
  //ZInteractiveContext *m_context;
  //mutable ZSharedPointer<ZStackDoc> m_doc;
};

///////////////
class ZKeyPressEventMapper : public ZKeyEventMapper
{
public:
  ZStackOperator getOperation(const QKeyEvent &event) const;
};

#endif // ZKEYEVENTMAPPER_H
