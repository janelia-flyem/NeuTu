#ifndef ZSTACKWATERSHEDCONTAINER_H
#define ZSTACKWATERSHEDCONTAINER_H

#include "tz_stack_watershed.h"
#include "zintcuboid.h"

class ZStack;
class ZObject3dScan;
class ZStroke2d;
class ZObject3d;
class ZSparseStack;

class ZStackWatershedContainer
{
public:
  ZStackWatershedContainer(ZStack *stack);
  ZStackWatershedContainer(ZSparseStack *stack);
  ~ZStackWatershedContainer();

  ZStack* run();

  void addSeed(const ZStack &seed);
  void addSeed(const ZObject3dScan &seed);
  void addSeed(const ZStroke2d &seed);
  void addSeed(const ZObject3d &seed);


  //It will remove old seeds


  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);

  void exportMask(const std::string &filePath);

private:
  void init();

  Stack_Watershed_Workspace* getWorkspace();
  void clearWorkspace();
  void clearSource();
  Stack* getSource();

  ZIntPoint getSourceOffset() const;

  Stack* getSeedMask();

  void prepareSeedMask(Stack *stack, Stack *mask);

  void makeMaskStack(ZStack &stack);

  ZIntPoint getSourceDsIntv() const;

private:
  ZStack *m_stack;
  ZSparseStack *m_spStack;
  Stack_Watershed_Workspace *m_workspace;
  ZIntPoint m_sourceOffset;
  ZIntCuboid m_range;
  ZStack *m_source;
  bool m_floodingZero;
  int m_channel;
};

#endif // ZSTACKWATERSHEDCONTAINER_H
