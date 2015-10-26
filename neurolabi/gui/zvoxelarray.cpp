#include "zvoxelarray.h"

#include <algorithm>

#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "swctreenode.h"
#include "tz_geo3d_utils.h"
#include "zswctree.h"
#include "swc/zswcresampler.h"
#include "neutubeconfig.h"

using namespace std;

ZVoxelArray::ZVoxelArray()
{
}

void ZVoxelArray::append(const ZVoxel &voxel)
{
  m_voxelArray.push_back(voxel);
}

void ZVoxelArray::append(int x, int y, int z, double value)
{
  m_voxelArray.push_back(ZVoxel(x, y, z, value));
}

void ZVoxelArray::prepend(const ZVoxel &voxel)
{
  m_voxelArray.insert(m_voxelArray.begin(), voxel);
}

void ZVoxelArray::addValue(double delta)
{
  for (vector<ZVoxel>::iterator iter = m_voxelArray.begin();
       iter != m_voxelArray.end(); ++iter) {
    (*iter).setValue((*iter).value() + delta);
  }
}

void ZVoxelArray::minimizeValue(double v)
{
  for (vector<ZVoxel>::iterator iter = m_voxelArray.begin();
       iter != m_voxelArray.end(); ++iter) {
    iter->setValue(min(iter->value(), v));
  }
}

void ZVoxelArray::labelStack(Stack *stack, double value) const
{
  for (vector<ZVoxel>::const_iterator iter = m_voxelArray.begin(); iter != m_voxelArray.end(); ++iter) {
    if ((*iter).isInBound(Stack_Width(stack), Stack_Height(stack),
                          Stack_Depth(stack))) {
      Set_Stack_Pixel(stack, (*iter).x(), (*iter).y(), (*iter).z(), 0, value);
    }
  }
}

void ZVoxelArray::setStackValue(Stack *stack) const
{
  for (vector<ZVoxel>::const_iterator iter = m_voxelArray.begin(); iter != m_voxelArray.end(); ++iter) {
    if ((*iter).isInBound(Stack_Width(stack), Stack_Height(stack),
                          Stack_Depth(stack))) {
      Set_Stack_Pixel(stack, (*iter).x(), (*iter).y(), (*iter).z(), 0,
                      (*iter).value());
    }
  }
}

void ZVoxelArray::sample(const Stack *stack)
{
  for (vector<ZVoxel>::iterator iter = m_voxelArray.begin(); iter != m_voxelArray.end(); ++iter) {
    if ((*iter).isInBound(Stack_Width(stack), Stack_Height(stack),
                          Stack_Depth(stack))) {
      (*iter).sample(stack);
    }
  }
}

void ZVoxelArray::sample(const Stack *stack, double (*f)(double))
{
  for (vector<ZVoxel>::iterator iter = m_voxelArray.begin(); iter != m_voxelArray.end(); ++iter) {
    if ((*iter).isInBound(Stack_Width(stack), Stack_Height(stack),
                          Stack_Depth(stack))) {
      (*iter).sample(stack, f);
    }
  }
}

void ZVoxelArray::labelStackWithBall(Stack *stack, double value) const
{
  for (vector<ZVoxel>::const_iterator iter = m_voxelArray.begin(); iter != m_voxelArray.end(); ++iter) {
    if ((*iter).isInBound(Stack_Width(stack), Stack_Height(stack),
                          Stack_Depth(stack))) {
      (*iter).labelStackWithBall(stack, value);
    }
  }
}

Swc_Tree* ZVoxelArray::toSwcTree() const
{
  if (m_voxelArray.empty()) {
    return NULL;
  }

  return toSwcTree(0, size() - 1);
}

Swc_Tree* ZVoxelArray::toSwcTree(size_t startIndex, size_t endIndex) const
{
  if (startIndex > endIndex) {
    return NULL;
  }

  if (endIndex >= size()) {
    endIndex = size() - 1;
  }

  ZSwcTree treeWrapper;

  Swc_Tree *tree = New_Swc_Tree();

  ZVoxel prevVoxel = m_voxelArray[startIndex];

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  SwcTreeNode::setPos(tn, prevVoxel.x(), prevVoxel.y(), prevVoxel.z());
  SwcTreeNode::setRadius(tn, prevVoxel.value());

  Swc_Tree_Node *prevTn = tn;

  for (size_t i = startIndex + 1; i < endIndex; i++) {
    //double dist = at(i).distanceTo(prevVoxel);
    bool sampling = true;
#if 0
    if ((dist > prevVoxel.value()) && (dist > at(i).value())) {
      if (dist > prevVoxel.value() + at(i).value()) {
        sampling = true;
      } else {
        double minValue = std::min(prevVoxel.value(), at(i).value());
        double maxValue = std::max(prevVoxel.value(), at(i).value());
        if (maxValue >= minValue + minValue + 1.0) {
          sampling = true;
        }
      }
    }/* else if (dist > prevVoxel.value()) {
      if (prevVoxel.value() < at(i).value()) {
        sampling = true;
      }
    }*/ //for further development
#endif
    if (sampling) {
      tn = New_Swc_Tree_Node();

      SwcTreeNode::setPos(tn, m_voxelArray.at(i).x(), m_voxelArray.at(i).y(), m_voxelArray.at(i).z());
      SwcTreeNode::setRadius(tn, m_voxelArray.at(i).value());
      Swc_Tree_Node_Set_Parent(prevTn, tn);
      prevTn = tn;
      prevVoxel = m_voxelArray.at(i);
    }
  }

  if (endIndex - startIndex > 0) {
    tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, m_voxelArray[endIndex].x(), m_voxelArray[endIndex].y(),
                        m_voxelArray[endIndex].z());
    SwcTreeNode::setRadius(tn, m_voxelArray[endIndex].value());
    Swc_Tree_Node_Set_Parent(prevTn, tn);
    if (SwcTreeNode::hasOverlap(prevTn, tn) && SwcTreeNode::hasChild(prevTn)) {
      SwcTreeNode::mergeToParent(prevTn);
    }
  }

  tree->root = tn;

#ifdef _DEBUG_2
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree((GET_TEST_DATA_DIR + "/test.swc").c_str(), tree);
#endif

  treeWrapper.setData(tree);
  ZSwcResampler sampler;
  sampler.ignoreInterRedundant(true);
  sampler.optimalDownsample(&treeWrapper);
  treeWrapper.setData(NULL, ZSwcTree::LEAVE_ALONE);

#ifdef _DEBUG_2
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree((GET_TEST_DATA_DIR + "/test2.swc").c_str(), tree);
#endif


  return tree;
}

size_t ZVoxelArray::findClosest(double x, double y)
{
  size_t targetIndex = 0;
  if (!m_voxelArray.empty()) {
    double minDist = Geo3d_Dist_Sqr(
          m_voxelArray.front().x(), m_voxelArray.front().y(), 0, x, y, 0);
    for (size_t i = 0; i < size(); ++i) {
      double dist = Geo3d_Dist_Sqr(m_voxelArray.at(i).x(), m_voxelArray.at(i).y(), 0, x, y, 0);
      if (dist < minDist) {
        minDist = dist;
        targetIndex = i;
      }
    }
  }

  return targetIndex;
}

size_t ZVoxelArray::findClosest(double x, double y, double z)
{
  size_t targetIndex = 0;
  if (!m_voxelArray.empty()) {
    double minDist = Geo3d_Dist_Sqr(
          m_voxelArray.front().x(), m_voxelArray.front().y(), m_voxelArray.front().z(), x, y, z);
    for (size_t i = 0; i < size(); ++i) {
      double dist = Geo3d_Dist_Sqr(m_voxelArray.at(i).x(), m_voxelArray.at(i).y(), m_voxelArray.at(i).z(), x, y, z);
      if (dist < minDist) {
        minDist = dist;
        targetIndex = i;
      }
    }
  }

  return targetIndex;
}

double ZVoxelArray::getCurveLength() const
{
  double length = 0.0;

  for (size_t i = 1; i < size(); ++i) {
    length += m_voxelArray.at(i).distanceTo(m_voxelArray.at(i - 1));
  }

  return length;
}

void ZVoxelArray::print() const
{
  std::cout << size() << " voxels" << std::endl;
  for (size_t i = 0; i < size(); ++i) {
    m_voxelArray[i].print();
  }
}
