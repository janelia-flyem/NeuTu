#ifndef ZKEYOPERATION_H
#define ZKEYOPERATION_H


class ZKeyOperation
{
public:
  ZKeyOperation();

  enum EGroup {
    OG_STACK, OG_STACK_3D,
    OG_STACK_OBJECT,
    OG_SWC_TREE,
    OG_SWC_TREE_NODE,
    OG_STROKE,
    OG_OTHER
  };
};

#endif // ZKEYOPERATION_H
