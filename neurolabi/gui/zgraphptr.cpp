#include "zgraphptr.h"

ZGraphPtr::ZGraphPtr()
{
}


ZGraphPtr::ZGraphPtr(ZGraph *g) : ZSharedPointer<ZGraph>(g)
{

}

ZGraphPtr ZGraphPtr::Make(ZGraph::EGraphType type)
{
  return ZGraphPtr(new ZGraph(type));
}
