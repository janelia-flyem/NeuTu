#ifndef ZSTACKDOCLABELSTACKFACTORY_H
#define ZSTACKDOCLABELSTACKFACTORY_H

#include "zstackfactory.h"


class ZStackDoc;

class ZStackDocLabelStackFactory : public ZStackFactory
{
public:
  ZStackDocLabelStackFactory();

  ZStack *makeStack(ZStack *stack = NULL) const;

  inline void setDocument(ZStackDoc *doc) {
    m_doc = doc;
  }

  inline const ZStackDoc* getDocument() const {
    return m_doc;
  }

private:
  ZStackDoc *m_doc;
};

#endif // ZSTACKDOCLABELSTACKFACTORY_H
