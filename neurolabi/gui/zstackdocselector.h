#ifndef ZSTACKDOCSELECTOR_H
#define ZSTACKDOCSELECTOR_H

#include <map>

#include "zsharedpointer.h"
#include "zstackobject.h"

class ZStackDoc;

class ZStackDocSelector
{
public:
  ZStackDocSelector(const ZSharedPointer<ZStackDoc> &doc);

  enum ESelectOption {
    SELECT_IGNORE, SELECT_NORMAL, SELECT_RECURSIVE
  };

  void deselectAll();

  void setSelectOption(ZStackObject::EType type, ESelectOption option);

private:
  ZSharedPointer<ZStackDoc> m_doc;
  std::map<ZStackObject::EType, ESelectOption> m_option;
};

#endif // ZSTACKDOCSELECTOR_H
