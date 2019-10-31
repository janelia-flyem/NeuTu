#ifndef ZFLYEMTODODELEGATE_H
#define ZFLYEMTODODELEGATE_H

#include "common/neutudefs.h"

class ZStackDoc;

class ZFlyEmToDoDelegate
{
public:
  ZFlyEmToDoDelegate(ZStackDoc *doc);
  ~ZFlyEmToDoDelegate() = default;

public:
  void add(int x, int y, int z, bool checked, neutu::EToDoAction action,
           uint64_t bodyId);

private:
  ZStackDoc *m_doc = NULL;
};

#endif // ZFLYEMTODODELEGATE_H
