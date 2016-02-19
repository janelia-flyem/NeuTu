#ifndef ZKEYEVENTMAPPERFACTORY_H
#define ZKEYEVENTMAPPERFACTORY_H

class ZKeyEventMapper;

class ZKeyEventMapperFactory
{
public:
  ZKeyEventMapperFactory();

  static ZKeyEventMapper* MakeSwcNodeMapper();
};

#endif // ZKEYEVENTMAPPERFACTORY_H
