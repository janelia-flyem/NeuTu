#ifndef ZKEYEVENTMAPPER_H
#define ZKEYEVENTMAPPER_H

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
  /*
  ZKeyEventMapper(ZInteractiveContext *context = NULL,
                  ZStackDoc *doc = NULL);
                  */

  virtual ZStackOperator getOperation(const QKeyEvent &event) const;

  ZStackOperator initOperation() const;
/*
  inline void setContext(ZInteractiveContext *context) {
    m_context = context;
  }

  inline void setDocument(ZSharedPointer<ZStackDoc> doc) {
    m_doc = doc;
  }
*/
protected:
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
