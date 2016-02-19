#ifndef ZSWCTREENODESELECTOR_H
#define ZSWCTREENODESELECTOR_H

#include "swctreenode.h"
#include "zselector.h"

class ZSwcTreeNodeSelector : public ZSelector<Swc_Tree_Node*>
{
public:
  ZSwcTreeNodeSelector();

  void print() const;
};

#endif // ZSWCTREENODESELECTOR_H
