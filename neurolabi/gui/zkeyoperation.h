#ifndef ZKEYOPERATION_H
#define ZKEYOPERATION_H


class ZKeyOperation
{
public:
  ZKeyOperation();

  enum EGroup {
    OG_ACTIVE_STROKE,
    OG_STACK,
    OG_STACK_3D,
    OG_STACK_OBJECT,
    OG_SWC_TREE,
    OG_SWC_TREE_NODE,
    OG_STROKE,
    OG_FLYEM_BOOKMARK,
    OG_OTHER
  };
};

#endif // ZKEYOPERATION_H
