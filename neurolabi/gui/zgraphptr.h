#ifndef ZGRAPHPTR_H
#define ZGRAPHPTR_H

#include "zsharedpointer.h"
#include "zgraph.h"

class ZGraphPtr : public ZSharedPointer<ZGraph>
{
public:
  ZGraphPtr();
  ZGraphPtr(ZGraph *graph);

  static ZGraphPtr Make(ZGraph::EGraphType type);
};

#endif // ZGRAPHPTR_H
