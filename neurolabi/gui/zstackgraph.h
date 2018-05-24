#ifndef ZSTACKGRAPH_H
#define ZSTACKGRAPH_H

#include <vector>
#include "tz_image_lib_defs.h"
#include "tz_stack_graph.h"

class ZGraph;
class ZIntCuboid;

/*!
 * \brief The ZStackGraph class provides functions to perform graph processing
 *        of a stack.
 *
 *
 *
 */
class ZStackGraph
{
public:
  ZStackGraph();
  ~ZStackGraph();

public:
  /*!
   * \brief Build a graph of a stack
   *
   * buildGraph() constructs a graph of \a stack. Each vertex of the graph is
   * a voxel index and each edge of the graph represents the neighboring
   * relationship of two voxels.
   *
   * \param stack Signal stack
   * \return The graph of voxels
   */
  ZGraph* buildGraph(const Stack *stack);

  /*!
   * \brief Build a graph of the foreground of the stack
   *
   * This function only checks pixels with intensity greater than 0. It also
   * ignores the current signal mask.
   *
   * \param stack Input stack
   */
  ZGraph* buildForegroundGraph(const Stack *stack);


  /*!
   * \brief Build a graph of the surface of the objects in the stack
   * \param stack Must be binary stack. The function does check it.
   *
   */
  ZGraph* buildSurfaceGraph(const Stack *stack);

  /*!
   * \brief Set the mask of the signal
   *
   * The function copies \a mask without owning it.
   *
   * \param mask Signal mask, which must be GREY kind.
   */
  void setSignalMask(const Stack *mask);

  /*!
   * \brief Set signal mask from a 2D image by projecting it through Z
   *
   * Each slice of the signal mask will be the same as the first slice of \a mask.
   *
   * \param mask Mask image.
   * \param depth Depth of the mask to create. It must be same as the depth of
   *        the input stack for building the graph.
   */
  void setSignalMaskAcrossZ(const Stack *mask, int depth);

  /*!
   * \brief Set the group mask
   *
   * It copies \a mask without owning it.
   *
   * \param mask Group mask.
   */
  void setGroupMask(const Stack *mask);

  /*!
   * \brief Set group mask from a 2D image by projecting it through Z
   *
   * Each slice of the group mask will be the same as the first slice of \a mask.
   *
   * \param mask Mask image.
   * \param depth Depth of the mask to create. It must be same as the depth of
   *        the input stack for building the graph.
   */
  void setGroupMaskAcrossZ(const Stack *mask, int depth);

  //ZGraph* buildGraph(const Stack *stack, const Stack *mask);

  int getGroupId(int voxelIndex);
  void setWeightFunction(Weight_Func_t f);

  //untested
  enum EVertexOption {
    VO_ALL, VO_FOREGROUND, VO_SURFACE
  };

  std::vector<int> computeShortestPath(const Stack *stack,
                                       int startIndex, int endIndex,
                                       EVertexOption option = VO_ALL);

  //untested
  void updateRange(size_t startIndex, size_t endIndex,
                   int width, int height, int depth);
  void updateRange(int x1, int y1, int z1, int x2, int y2, int z2,
                   int width, int height, int depth);
  void setRange(int x1, int y1, int z1, int x2, int y2, int z2);

  ZIntCuboid getRange();

  size_t getRoiVolume() const;

  //untested
  void inferWeightParameter(const Stack *stack);

  void setResolution(const double *res);

  void setZMargin(int m) { m_zMargin = m; }

private:
  void initRange(const Stack *stack, int *range);

private:
  Stack_Graph_Workspace m_workspace;
  int m_zMargin;
};

#endif // ZSTACKGRAPH_H
