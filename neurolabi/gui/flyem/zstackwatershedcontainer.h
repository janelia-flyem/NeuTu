#ifndef ZSTACKWATERSHEDCONTAINER_H
#define ZSTACKWATERSHEDCONTAINER_H

#include <utility>

#include "tz_stack_watershed.h"
#include "zintcuboid.h"

class ZStack;
class ZObject3dScan;
class ZStroke2d;
class ZObject3d;
class ZSparseStack;
class ZObject3dScanArray;

class ZStackWatershedContainer
{
public:
  ZStackWatershedContainer(ZStack *stack);
  ZStackWatershedContainer(ZSparseStack *stack);
  ZStackWatershedContainer(ZStack *stack, ZSparseStack *spStack);
  ZStackWatershedContainer(const std::pair<ZStack*, ZSparseStack*> &data);

  ~ZStackWatershedContainer();

  bool isEmpty() const;

  void run();

  void addSeed(const ZStack &seed);
  void addSeed(const ZObject3dScan &seed);
  void addSeed(const ZStroke2d &seed);
  void addSeed(const ZObject3d &seed);


  //It will remove old seeds


  void setRange(const ZIntCuboid &range);
  void setRange(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);

  void exportMask(const std::string &filePath);
  void exportSource(const std::string &filePath);

  void setFloodFillingZero(bool on) {
    m_floodingZero = on;
  }

  ZStack* getResultStack() const {
    return m_result;
  }

  ZObject3dScanArray* makeSplitResult(ZObject3dScanArray *result = NULL);

  void printInfo() const;

private:
  void init();
  void init(ZStack *stack, ZSparseStack *spStack);

  Stack_Watershed_Workspace* getWorkspace();
  void clearWorkspace();
  void clearSource();
  void clearResult();
  Stack* getSource();
  ZStack* getSourceStack();

  ZIntPoint getSourceOffset() const;

  Stack* getSeedMask();

  void prepareSeedMask(Stack *stack, Stack *mask);

  void makeMaskStack(ZStack &stack);

  ZIntPoint getSourceDsIntv();

private:
  ZStack *m_stack;
  ZSparseStack *m_spStack;
  ZStack *m_result;
  Stack_Watershed_Workspace *m_workspace;
  ZIntPoint m_sourceOffset;
  ZIntCuboid m_range;
  ZStack *m_source;
  bool m_floodingZero;
  int m_channel;
};

#endif // ZSTACKWATERSHEDCONTAINER_H
