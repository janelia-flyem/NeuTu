#include "zflyembodyanalyzer.h"
#include "zstackskeletonizer.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "swc/zswcresampler.h"

ZFlyEmBodyAnalyzer::ZFlyEmBodyAnalyzer()
{
  for (int i = 0; i < 3; ++i) {
    m_downsampleInterval[i] = 0;
  }
}

ZPointArray ZFlyEmBodyAnalyzer::computeHoleCenter(const ZObject3dScan &obj)
{
  ZObject3dScan bufferObj = obj;

  bufferObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
      m_downsampleInterval[2]);

  ZPointArray centerArray;

  std::vector<ZObject3dScan> objArray = bufferObj.findHoleObjectArray();
  for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    centerArray.push_back(iter->getCentroid());
  }

  centerArray.scale(m_downsampleInterval[0] + 1,
      m_downsampleInterval[1] + 1, m_downsampleInterval[2] + 1);

  return centerArray;
}

void ZFlyEmBodyAnalyzer::setDownsampleInterval(int ix, int iy, int iz)
{
  m_downsampleInterval[0] = ix;
  m_downsampleInterval[1] = iy;
  m_downsampleInterval[2] = iz;
}

ZPointArray ZFlyEmBodyAnalyzer::computeTerminalPoint(const ZObject3dScan &obj)
{
  ZObject3dScan bufferObj = obj;

  bufferObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
      m_downsampleInterval[2]);

  int offset[3] = {0, 0, 0};
  Stack *stack = bufferObj.toStack(offset);

  ZStackSkeletonizer skeletonizer;
  skeletonizer.setRebase(true);

  skeletonizer.setMinObjSize(100);
  skeletonizer.setDistanceThreshold(0);
  skeletonizer.setLengthThreshold(50);
  skeletonizer.setKeepingSingleObject(true);

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stack);

  C_Stack::kill(stack);

  if (wholeTree != NULL) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);
  }

  std::vector<Swc_Tree_Node*> terminalArray =
      wholeTree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  ZPointArray pts;
  pts.resize(terminalArray.size());

  for (size_t i = 0; i < terminalArray.size(); ++i) {
    pts[i] = SwcTreeNode::pos(terminalArray[i]);
  }
  pts.translate(ZPoint(offset[0], offset[1], offset[2]));
  pts.scale(m_downsampleInterval[0] + 1, m_downsampleInterval[1] + 1,
      m_downsampleInterval[2] + 1);

  return pts;
}
