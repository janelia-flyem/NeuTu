#include "zswcpath.h"

#include <iostream>
#include <algorithm>
#include <cassert>

#include "neurolabi/numericarray.h"
#include "common/math.h"
#include "swctreenode.h"
#include "zswctree.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_int_arraylist.h"

using namespace std;

ZSwcPath::ZSwcPath() : m_hostTree(NULL)
{
}

ZSwcPath::ZSwcPath(Swc_Tree_Node *beginTn, Swc_Tree_Node *endTn) :
  m_hostTree(NULL)
{
  if (beginTn == NULL && endTn == NULL) {
    return;
  }

  Swc_Tree_Node *ancestor = NULL;

  if (beginTn == NULL) {
    ancestor = SwcTreeNode::regularRoot(endTn);
    beginTn = ancestor;
  } else if (endTn == NULL) {
    ancestor = SwcTreeNode::regularRoot(beginTn);
    endTn = ancestor;
  } else {
    ancestor = SwcTreeNode::commonAncestor(beginTn, endTn);
  }

  if (SwcTreeNode::isVirtual(ancestor)) {
    return;
  }

  Swc_Tree_Node *tn = beginTn;
  while (tn != ancestor) {
    push_back(tn);
    tn = tn->parent;
  }

  push_back(ancestor);

  if (endTn != ancestor) {
    vector<Swc_Tree_Node*> subPath;
    tn = endTn;
    while (tn != ancestor) {
      subPath.push_back(tn);
      tn = tn->parent;
    }
    for (vector<Swc_Tree_Node*>::reverse_iterator iter = subPath.rbegin();
         iter != subPath.rend(); ++iter) {
      push_back(*iter);
    }
  }
}

void ZSwcPath::label(int v)
{
  for (ZSwcPath::iterator iter = begin(); iter != end(); ++iter) {
    SwcTreeNode::setLabel(*iter, v);
  }
}

void ZSwcPath::setType(int type)
{
  for (ZSwcPath::iterator iter = begin(); iter != end(); ++iter) {
    SwcTreeNode::setType(*iter, type);
  }
}

void ZSwcPath::addLabel(int dv)
{
  for (ZSwcPath::iterator iter = begin(); iter != end(); ++iter) {
    SwcTreeNode::addLabel(*iter, dv);
  }
}

void ZSwcPath::print()
{
  for (ZSwcPath::iterator iter = begin(); iter != end(); ++iter) {
    cout << SwcTreeNode::toString(*iter) << endl;
  }
}

void ZSwcPath::reverse()
{
  std::reverse(begin(), end());
}

bool ZSwcPath::isEmpty() const
{
  return empty();
}

void ZSwcPath::append(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    push_back(tn);
  }
}

bool ZSwcPath::isContinuous() const
{
  if (isEmpty()) {
    return false;
  }

  for (size_t i = 1; i < size(); ++i) {
    if (!SwcTreeNode::isConnected((*this)[i - 1], (*this)[i])) {
      return false;
    }
  }

  return true;
}

void ZSwcPath::smooth(bool fixingTerminal)
{
  vector<double> xArray(size());
  vector<double> yArray(size());
  vector<double> zArray(size());
  vector<double> rArray(size());

  for (size_t index = 0; index < size(); ++index) {
    const Swc_Tree_Node *tn = (*this)[index];
    xArray[index] = SwcTreeNode::x(tn);
    yArray[index] = SwcTreeNode::y(tn);
    zArray[index] = SwcTreeNode::z(tn);
    rArray[index] = SwcTreeNode::radius(tn);
  }

  vector<double> xSmoothed(size());
  vector<double> ySmoothed(size());
  vector<double> zSmoothed(size());
  vector<double> rSmoothed(size());

  neutu::array::MedianFilter(&(xArray[0]), size(), 1, &(xSmoothed[0]));
  neutu::array::AverageSmooth(&(xSmoothed[0]), size(), 3, &(xArray[0]));

  neutu::array::MedianFilter(&(yArray[0]), size(), 1, &(ySmoothed[0]));
  neutu::array::AverageSmooth(&(ySmoothed[0]), size(), 3, &(yArray[0]));

  neutu::array::MedianFilter(&(zArray[0]), size(), 1, &(zSmoothed[0]));
  neutu::array::AverageSmooth(&(zSmoothed[0]), size(), 3, &(zArray[0]));

//  darray_medfilter(&(rArray[0]), size(), 1, &(rSmoothed[0]));
//  darray_avgsmooth(&(rSmoothed[0]), size(), 3, &(rArray[0]));


  for (size_t index = 0; index < size(); ++index) {
    Swc_Tree_Node *tn = (*this)[index];
    if (!fixingTerminal || SwcTreeNode::isContinuation(tn)) {
      SwcTreeNode::setX(tn, xArray[index]);
      SwcTreeNode::setY(tn, yArray[index]);
    }
    SwcTreeNode::setZ(tn, zArray[index]);
    SwcTreeNode::setRadius(tn, rArray[index]);
  }
}

void ZSwcPath::smoothZ()
{
  vector<double> zArray(size());
  for (size_t index = 0; index < size(); ++index) {
    zArray[index] = SwcTreeNode::z((*this)[index]);
  }

  vector<double> smoothed(size());

  neutu::array::MedianFilter(&(zArray[0]), size(), 5, &(smoothed[0]));
  neutu::array::AverageSmooth(&(smoothed[0]), size(), 3, &(zArray[0]));


  for (size_t index = 0; index < size(); ++index) {
    SwcTreeNode::setZ((*this)[index], zArray[index]);
  }
}

void ZSwcPath::smoothRadius(bool excludingBranchPoint)
{
  vector<double> valueArray(size());
  for (size_t index = 0; index < size(); ++index) {
    valueArray[index] = SwcTreeNode::radius((*this)[index]);
  }

  vector<double> smoothed(size());

  neutu::array::MedianFilter(&(valueArray[0]), size(), 5, &(smoothed[0]));
  neutu::array::AverageSmooth(&(smoothed[0]), size(), 3, &(valueArray[0]));


  for (size_t index = 0; index < size(); ++index) {
    if (!(excludingBranchPoint && SwcTreeNode::isBranchPoint((*this)[index]))) {
      SwcTreeNode::setRadius((*this)[index], valueArray[index]);
    }
  }
}

void ZSwcPath::labelStack(Stack *stack, int value)
{
  if (!empty()) {
    Swc_Tree_Node_Label_Workspace workspace;
    Default_Swc_Tree_Node_Label_Workspace(&workspace);
    workspace.sdw.color.r = value;

    workspace.label_mode = SWC_TREE_LABEL_NODE;
    Swc_Tree_Node_Label_Stack(front(), stack, &workspace);


    workspace.label_mode = SWC_TREE_LABEL_ALL;
    for (size_t index = 1; index < size() - 1; ++index) {
      Swc_Tree_Node_Label_Stack(at(index), stack, &workspace);
    }
#if 1
    if (size() > 1) {
      workspace.label_mode = SWC_TREE_LABEL_NODE;
      Swc_Tree_Node_Label_Stack(back(), stack, &workspace);

      workspace.label_mode = SWC_TREE_LABEL_CONNECTION;
      if (SwcTreeNode::parent(front()) == at(1)) {
        Swc_Tree_Node_Label_Stack(front(), stack, &workspace);
      } else if (SwcTreeNode::parent(back()) == at(size() - 2)) {
        Swc_Tree_Node_Label_Stack(back(), stack, &workspace);
      }
    }
#endif
  }
}

void ZSwcPath::labelStackAcrossZ(Stack *stack, int value)
{
  assert(stack != NULL);
  assert(C_Stack::kind(stack) == GREY);

  if (!empty()) {
    Swc_Tree_Node_Label_Workspace workspace;
    Default_Swc_Tree_Node_Label_Workspace(&workspace);
    workspace.z_proj = _TRUE_;
    workspace.sdw.color.r = value;

    workspace.label_mode = SWC_TREE_LABEL_NODE;
    Swc_Tree_Node_Label_Stack(front(), stack, &workspace);

    workspace.label_mode = SWC_TREE_LABEL_ALL;
    for (size_t index = 1; index < size() - 1; ++index) {
      Swc_Tree_Node_Label_Stack(at(index), stack, &workspace);
    }

    if (size() > 1) {
      workspace.label_mode = SWC_TREE_LABEL_NODE;
      Swc_Tree_Node_Label_Stack(back(), stack, &workspace);

      workspace.label_mode = SWC_TREE_LABEL_CONNECTION;
      if (SwcTreeNode::parent(front()) == at(1)) {
        Swc_Tree_Node_Label_Stack(front(), stack, &workspace);
      } else if (SwcTreeNode::parent(back()) == at(size() - 2)) {
        Swc_Tree_Node_Label_Stack(back(), stack, &workspace);
      }
    }

    size_t offset = 0;
    size_t deepOffset = 0;
    size_t area = C_Stack::area(stack);
    uint8_t *array = C_Stack::array8(stack);
    for (int y = 0; y < C_Stack::height(stack); ++y) {
      for (int x = 0; x < C_Stack::width(stack); ++x) {
        deepOffset = offset;
        if (array[offset++] == value) {
          for (int z = 1; z < C_Stack::depth(stack); ++z) {
            deepOffset += area;
            array[deepOffset] = value;
          }
        }
      }
    }
  }
}

void ZSwcPath::resetPositionFromStack(const Stack *stack)
{
  int sourceX = neutu::iround(SwcTreeNode::x(front()));
  int sourceY = neutu::iround(SwcTreeNode::y(front()));

  int targetX = neutu::iround(SwcTreeNode::x(back()));
  int targetY = neutu::iround(SwcTreeNode::y(back()));

  if (sourceX != targetX || sourceY != targetY) {

    Stack *mask = C_Stack::make(GREY, C_Stack::width(stack),
                                C_Stack::height(stack), C_Stack::depth(stack));

    Zero_Stack(mask);
    uint8_t barrier = SP_GROW_BARRIER;
    Stack_Set_Constant(mask, &barrier);

    labelStackAcrossZ(mask, 0);
    Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
    Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
    sgw->wf = Stack_Voxel_Weight;

    uint8_t *array = C_Stack::array8(mask);
    size_t area = C_Stack::area(mask);
    size_t sourceIndex = sourceX + C_Stack::width(mask) * sourceY;
    size_t targetIndex = targetX + C_Stack::width(mask) * targetY;
    for (int z = 0; z < C_Stack::depth(stack); ++z) {
      array[sourceIndex] = SP_GROW_SOURCE;
      array[targetIndex] = SP_GROW_TARGET;
      sourceIndex += area;
      targetIndex += area;
    }

    Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
    ZVoxelArray voxelArray;

    for (int i = 0; i < path->length; ++i) {
      int x, y, z;
      C_Stack::indexToCoord(path->array[i], C_Stack::width(stack), C_Stack::height(stack),
                            &x, &y, &z);
      bool isRedundant = false;
      if (mask->array[path->array[i]] == SP_GROW_SOURCE) {
        if (i < path->length - 1) {
          if (mask->array[i + 1] == SP_GROW_SOURCE) {
            isRedundant = true;
          }
        }
      }
      if (!isRedundant) {
        voxelArray.append(ZVoxel(x, y, z));
      }
    }
    Kill_Int_Arraylist(path);

    size_t currentIndex = 0;
    const std::vector<ZVoxel> &voxelData = voxelArray.getInternalData();
    for (iterator iter = begin(); iter != end(); ++iter) {
      size_t index = voxelArray.findClosest(
            SwcTreeNode::x(*iter), SwcTreeNode::y(*iter));
      if (index > currentIndex || currentIndex == 0) {
        SwcTreeNode::setPos(*iter, voxelData[index].x(), voxelData[index].y(),
                            voxelData[index].z());
        currentIndex = index;
      }
    }

    C_Stack::kill(mask);
    Kill_Sp_Grow_Workspace(sgw);
  }

}

double ZSwcPath::getLength() const
{
  double length = 0.0;
  for (size_t i = 1; i < size(); ++i) {
    if (SwcTreeNode::isConnected((*this)[i - 1], (*this)[i])) {
      length += SwcTreeNode::distance((*this)[i - 1], (*this)[i]);
    }
  }

  return length;
}

double ZSwcPath::getAverageRadius() const
{
  double radius = 0.0;

  for (ZSwcPath::const_iterator iter = begin(); iter != end(); ++iter) {
    radius += SwcTreeNode::radius(*iter);
  }

  if (size() > 0) {
    radius /= size();
  }

  return radius;
}
