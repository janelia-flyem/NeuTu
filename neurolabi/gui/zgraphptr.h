#ifndef ZGRAPHPTR_H
#define ZGRAPHPTR_H

#include "core/zsharedpointer.h"
#include "core/zgraph.h"

class ZGraphPtr : public ZSharedPointer<ZGraph>
{
public:
  ZGraphPtr();
  ZGraphPtr(ZGraph *graph);

  static ZGraphPtr Make(ZGraph::EGraphType type);
};

#endif // ZGRAPHPTR_H
