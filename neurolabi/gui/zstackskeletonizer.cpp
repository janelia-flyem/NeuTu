#include "zstackskeletonizer.h"

#include <iostream>
#include <string.h>
#include "c_stack.h"
#include "zswctree.h"
#include "tz_stack_lib.h"
#include "tz_stack_bwmorph.h"
#include "tz_sp_grow.h"
#include "tz_stack_objlabel.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "zswcforest.h"
#include "swctreenode.h"
#include "zswcgenerator.h"
#include "tz_error.h"
#include "zstack.hxx"
#include "zobject3dscan.h"
#include "zerror.h"
#include "tz_math.h"
#include "swc/zswcresampler.h"
#include "tz_stack_threshold.h"

using namespace std;

ZStackSkeletonizer::ZStackSkeletonizer() : m_lengthThreshold(15.0),
  m_distanceThreshold(-1.0), m_rebase(false), m_interpolating(false),
  m_removingBorder(false), m_fillingHole(false), m_minObjSize(0),
  m_keepingSingleObject(false), m_level(-1), m_connectingBranch(true),
  m_usingOriginalSignal(false), m_resampleSwc(true), m_autoGrayThreshold(true),
  m_grayOp(0)
{
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
    m_downsampleInterval[i] = 0;
  }
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const ZStack &stack)
{
  ZSwcTree *tree = makeSkeleton(stack.c_stack());
  if (tree != NULL) {
    const ZIntPoint &pt = stack.getOffset();
    tree->translate(pt.getX(), pt.getY(), pt.getZ());
  }

  return tree;
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(
    const std::vector<ZStack*> &stackArray)
{
  ZSwcTree *wholeTree = new ZSwcTree;

  int count = 0;
  for (std::vector<ZStack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    const ZStack* stack = *iter;
    ZSwcTree *tree = makeSkeleton(*stack);
    if (!tree->isEmpty()) {
      wholeTree->merge(tree, true);
      ++count;
    } else {
      delete tree;
    }
  }

  if (m_connectingBranch && count > 1) {
    reconnect(wholeTree);
  }

  return wholeTree;
}

static double AdjustedDistanceWeight(double v)
{
  return dmax2(0.1, sqrt(v) - 0.5);
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const ZObject3dScan &obj)
{
  ZSwcTree *tree = NULL;
  if (!obj.isEmpty()) {
    ZObject3dScan newObj = obj;
    ZIntCuboid box = obj.getBoundBox();
    std::cout << "Downsampling " << m_downsampleInterval[0] + 1 << " x "
              << m_downsampleInterval[1] + 1 << " x "
              << m_downsampleInterval[2] + 1 << std::endl;
    newObj.downsampleMax(m_downsampleInterval[0],
                         m_downsampleInterval[1], m_downsampleInterval[2]);
    int offset[3] = {0, 0, 0};
    Stack *stack = newObj.toStack(offset);
    tree = makeSkeletonWithoutDs(stack);
    if (tree != NULL) {
      const ZIntPoint pt = box.getFirstCorner();
      tree->translate(pt.getX(), pt.getY(), pt.getZ());
    }
  }

  return tree;
}


ZSwcTree* ZStackSkeletonizer::makeSkeletonWithoutDsTest(Stack *stackData)
{
  Stack *signal = NULL;

  if (m_usingOriginalSignal) {
    signal = C_Stack::clone(stackData);
  }

  int level = m_level;
  if (m_autoGrayThreshold) {
    level = Stack_Threshold_RC(stackData, 0, 65535);
  }
//  if (m_level > 0) {
  Stack_Level_Mask(stackData, level);
//  }
  advanceProgress(0.05);

  double maxMaskIntensity = Stack_Max(stackData, NULL);
  if (maxMaskIntensity > 1.0) {
    Stack_Binarize(stackData);
  } else if (maxMaskIntensity == 0.0) {
    C_Stack::kill(stackData);
    cout << "Not a binary image. No skeleton generated." << endl;
    return NULL;
  }

  if (C_Stack::kind(stackData) != GREY) {
    Translate_Stack(stackData, GREY, 1);
  }

  advanceProgress(0.05);

  Stack *out = stackData;

  if (m_removingBorder) {
    cout << "Remove 1-pixel gaps ..." << endl;
    Stack_Not(out, out);
    Stack* solid = Stack_Majority_Filter(out, NULL, 8);
    Kill_Stack(out);

    Stack_Not(solid, solid);
    stackData = solid;
  }
  advanceProgress(0.05);

  if (m_interpolating) {
    cout << "Interpolating ..." << endl;
    out = Stack_Bwinterp(stackData, NULL);
    Kill_Stack(stackData);
    stackData = out;
  }
  advanceProgress(0.05);

  if (m_fillingHole) {
    std::cout << "Filling hole ..." << std::endl;
    Stack *bufferStack = Stack_Fill_Hole_N(stackData, NULL, 1, 26, NULL);
    C_Stack::kill(stackData);
    stackData = bufferStack;
  }

#ifdef _DEBUG_2
    C_Stack::write("/groups/flyem/home/zhaot/Work/neutube/neurolabi/data/test.tif", stackData);
#endif

  int dsVol = (m_downsampleInterval[0] + 1) * (m_downsampleInterval[1] + 1) *
      (m_downsampleInterval[2] + 1);

  cout << "Label objects ...\n" << endl;
  int minObjSize = m_minObjSize;
  minObjSize /= dsVol;
  int nobj = Stack_Label_Large_Objects_N(
        stackData, NULL, 1, 2, minObjSize, 26);
  //int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
  if (nobj == 0) {
    cout << "No object found in the image. No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }


  advanceProgress(0.1);

  if (nobj > 65533) {
    cout << "Too many objects ( > 65533). No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  size_t voxelNumber = C_Stack::voxelNumber(stackData);

  double step = 0.6 / nobj;
  for (int objIndex = 0; objIndex < nobj; objIndex++) {
    cout << "Skeletonizing object " << objIndex + 1 << "/" << nobj << endl;
    Swc_Tree *subtree = New_Swc_Tree();
    subtree->root = Make_Virtual_Swc_Tree_Node();

    Stack *objstack = Copy_Stack(stackData);
    size_t objSize = Stack_Level_Mask(objstack, 3 + objIndex);

    Translate_Stack(objstack, GREY, 1);

    int objectOffset[3];
    Stack *croppedObjStack = C_Stack::boundCrop(objstack, 0, objectOffset);

    if (signal != NULL) {
      Cuboid_I bound_box;
      Stack_Bound_Box(objstack, &bound_box);
      Stack *tmpStack = C_Stack::crop(signal, bound_box, NULL);
      C_Stack::kill(signal);
      signal = tmpStack;
    }

    /*
    if (C_Stack::kind(objstack) == GREY16) {
      Translate_Stack(objstack, GREY, 1);
    }
    */

    if (C_Stack::kind(croppedObjStack) == GREY16) {
      Translate_Stack(croppedObjStack, GREY, 1);
    }

    double linScale = 1.0;

    if (m_downsampleInterval[0] == m_downsampleInterval[1] &&
        m_downsampleInterval[1] == m_downsampleInterval[2]) {
      linScale = m_downsampleInterval[0] + 1;
    } else {
      linScale = Cube_Root(dsVol);
    }

    double lengthThreshold = m_lengthThreshold / linScale;
    if (objSize == 1) {
      if (m_keepingSingleObject || lengthThreshold <= 1) {
        int x = 0;
        int y = 0;
        int z = 0;
        for (size_t offset = 0; offset < voxelNumber; ++offset) {
          if (croppedObjStack->array[offset] == 1) {
            C_Stack::indexToCoord(offset, C_Stack::width(croppedObjStack),
                                  C_Stack::height(croppedObjStack), &x, &y, &z);
            break;
          }
        }
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              x + objectOffset[0], y + objectOffset[1],
            z + objectOffset[2], 1.0);
        SwcTreeNode::setParent(tn, subtree->root);
      }
    } else {
      cout << "Build distance map ..." << endl;
      Stack *tmpdist = Stack_Bwdist_L_U16P(croppedObjStack, NULL, 0);

      cout << "Shortest path grow ..." << endl;
      Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
      Sp_Grow_Workspace_Enable_Eucdist_Buffer(sgw);
      sgw->resolution[0] = m_resolution[0];
      sgw->resolution[1] = m_resolution[1];
      sgw->resolution[2] = m_resolution[2];
      sgw->wf = Stack_Voxel_Weight_I;
      size_t max_index;
      Stack_Max(tmpdist, &max_index);

      Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist),
                               Stack_Height(tmpdist),
                               Stack_Depth(tmpdist));
      Zero_Stack(mask);

      size_t nvoxel = Stack_Voxel_Number(croppedObjStack);
      size_t i;
      for (i = 0; i < nvoxel; i++) {
        if (croppedObjStack->array[i] == 0) {
          mask->array[i] = SP_GROW_BARRIER;
        }
      }

      mask->array[max_index] = SP_GROW_SOURCE;
      Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
      Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);

      ZSpGrowParser parser(sgw);

      if (m_rebase) {
        cout << "Replacing start point ..." << endl;
        ZVoxelArray path = parser.extractLongestPath(NULL, false);
        for (i = 0; i < nvoxel; i++) {
          if (mask->array[i] != SP_GROW_BARRIER) {
            mask->array[i] = 0;
          }
        }

        const std::vector<ZVoxel>& pathData = path.getInternalData();
        ssize_t seedIndex = pathData[0].toIndex(
              C_Stack::width(mask), C_Stack::height(mask),
              C_Stack::depth(mask));
        mask->array[seedIndex] = SP_GROW_SOURCE;
        if (signal != NULL) {
          Stack_Sp_Grow(signal, NULL, 0, NULL, 0, sgw);
        } else {
          Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
        }
      }
      //double lengthThreshold = lengthThreshold;

      std::vector<ZVoxelArray> pathArray =
          parser.extractAllPath(lengthThreshold, tmpdist);

      if (pathArray.empty() && m_keepingSingleObject) {
        pathArray.push_back(parser.extractLongestPath(NULL, false));
      }

      //Make a subtree from a single object
      for (std::vector<ZVoxelArray>::iterator iter = pathArray.begin();
           iter != pathArray.end(); ++iter) {
        (*iter).sample(tmpdist, AdjustedDistanceWeight);
        //(*iter).labelStack(stackData, 255.0);

        ZSwcTree *branchWrapper =
            ZSwcGenerator::createSwc(*iter, ZSwcGenerator::REGION_SAMPLING);
        Swc_Tree *branch  = branchWrapper->data();
        branchWrapper->setData(branch, ZSwcTree::LEAVE_ALONE);
        branchWrapper->translate(objectOffset[0], objectOffset[1],
            objectOffset[2]);

        //branch = (*iter).toSwcTree();
        if (SwcTreeNode::firstChild(branch->root) != NULL) {
          if (SwcTreeNode::radius(branch->root) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::firstChild(branch->root))) {
            Swc_Tree_Node *oldRoot = branch->root;
            Swc_Tree_Node *newRoot = SwcTreeNode::firstChild(branch->root);
            SwcTreeNode::detachParent(newRoot);
            branch->root = newRoot;
            SwcTreeNode::kill(oldRoot);
          }
        }

        Swc_Tree_Node *leaf = SwcTreeNode::firstChild(branch->root);
        if (leaf != NULL) {
          while (SwcTreeNode::firstChild(leaf) != NULL) {
            leaf = SwcTreeNode::firstChild(leaf);
          }

          if (SwcTreeNode::radius(leaf) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::parent(leaf))) {
            SwcTreeNode::detachParent(leaf);
            SwcTreeNode::kill(leaf);
          }
        }

        Swc_Tree_Node *tn = Swc_Tree_Connect_Branch(subtree, branch->root);

#ifdef _DEBUG_2
        if (SwcTreeNode::length(branch->root) > 100) {
          std::cout << "Potential bug." << std::endl;
        }
#endif

        if (SwcTreeNode::isRegular(SwcTreeNode::parent(tn))) {
          if (SwcTreeNode::hasOverlap(tn, SwcTreeNode::parent(tn))) {
            SwcTreeNode::mergeToParent(tn);
          }
        }

        branch->root = NULL;
        Kill_Swc_Tree(branch);
        branch = NULL;

#ifdef _DEBUG_2
        Swc_Tree_Iterator_Start(subtree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
        Swc_Tree_Node *tmptn = NULL;
        while ((tmptn = Swc_Tree_Next(subtree)) != NULL) {
          if (!SwcTreeNode::isRoot(tmptn)) {
            TZ_ASSERT(SwcTreeNode::length(tmptn) > 0.0, "duplicating nodes");
          }
        }

#endif
      }

      Kill_Stack(mask);
      Kill_Stack(tmpdist);
    }

    C_Stack::kill(croppedObjStack);
    croppedObjStack = NULL;
    C_Stack::kill(objstack);
    objstack = NULL;
    C_Stack::kill(signal);
    signal = NULL;

    if (Swc_Tree_Regular_Root(subtree) != NULL) {
#ifdef _DEBUG_
      cout << Swc_Tree_Node_Fsize(subtree->root) - 1<< " nodes added" << endl;
#endif
      Swc_Tree_Merge(tree, subtree);
    }

    Kill_Swc_Tree(subtree);

    advanceProgress(step);
  }

  C_Stack::kill(stackData);
  stackData = NULL;

  ZSwcTree *wholeTree = NULL;

  if (Swc_Tree_Regular_Root(tree) != NULL) {
    wholeTree = new ZSwcTree;
    wholeTree->setData(tree);

    if (m_downsampleInterval[0] > 0 || m_downsampleInterval[1] > 0 ||
        m_downsampleInterval[2] > 0) {
      wholeTree->rescale(m_downsampleInterval[0] + 1,
          m_downsampleInterval[1] + 1, m_downsampleInterval[2] + 1);
    }

    if (m_connectingBranch) {
      reconnect(wholeTree);
    }

    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);

    wholeTree->resortId();
  }

  advanceProgress(0.05);
  endProgress();

  return wholeTree;
}

ZSwcTree* ZStackSkeletonizer::makeSkeletonWithoutDs(Stack *stackData)
{
  Stack *stackSignal = NULL;

  /*
  if (m_level > 0) {
    Stack_Level_Mask(stackData, m_level);
  }
  */

  advanceProgress(0.05);

  double maxMaskIntensity = Stack_Max(stackData, NULL);

  if (maxMaskIntensity == 0.0) {
    C_Stack::kill(stackData);
    cout << "Not a binary image. No skeleton generated." << endl;
    return NULL;
  }


  if (m_grayOp >= 0) {
    switch (m_grayOp) {
    case 0: //>=
      Stack_Threshold(stackData, m_level);
      break;
    case 1: //<=
      Stack_Lower_Threshold(stackData, m_level);
      break;
    case 2: //==
      Stack_Level_Mask(stackData, m_level);
    }
  } else if (maxMaskIntensity > 1.0) {
    if (m_usingOriginalSignal) {
      stackSignal = C_Stack::clone(stackData);
    }

    int thre = 0;
    if (m_autoGrayThreshold) {
      int thre1 =
          Stack_Find_Threshold_RC(stackData, 0, iround(maxMaskIntensity));
      int thre2 = Stack_Find_Threshold_Locmax(
            stackData, 0, iround(maxMaskIntensity));

      thre = imax2(thre1, thre2);
    }
    Stack_Threshold(stackData, thre);
  }

  if (maxMaskIntensity > 1.0) {
    Stack_Binarize(stackData);
  }

  if (C_Stack::kind(stackData) != GREY) {
    Translate_Stack(stackData, GREY, 1);
  }

  advanceProgress(0.05);

  Stack *out = stackData;

  if (m_removingBorder) {
    cout << "Remove 1-pixel gaps ..." << endl;
    Stack_Not(out, out);
    Stack* solid = Stack_Majority_Filter(out, NULL, 8);
    Kill_Stack(out);

    Stack_Not(solid, solid);
    stackData = solid;
  }
  advanceProgress(0.05);

  if (m_interpolating) {
    cout << "Interpolating ..." << endl;
    out = Stack_Bwinterp(stackData, NULL);
    Kill_Stack(stackData);
    stackData = out;
  }
  advanceProgress(0.05);

  if (m_fillingHole) {
    std::cout << "Filling hole ..." << std::endl;
    Stack *bufferStack = Stack_Fill_Hole_N(stackData, NULL, 1, 26, NULL);
    C_Stack::kill(stackData);
    stackData = bufferStack;
  }

#ifdef _DEBUG_2
    C_Stack::write("/groups/flyem/home/zhaot/Work/neutube/neurolabi/data/test.tif", stackData);
#endif

  int dsVol = (m_downsampleInterval[0] + 1) * (m_downsampleInterval[1] + 1) *
      (m_downsampleInterval[2] + 1);

  cout << "Label objects ...\n" << endl;
  int minObjSize = m_minObjSize;
  minObjSize /= dsVol;

  Objlabel_Workspace ow;
  Default_Objlabel_Workspace(&ow);
  ow.conn = 26;
  ow.chord = NULL;
  ow.init_chord = TRUE;
  ow.inc_label = TRUE;

  int nobj = Stack_Label_Large_Objects_W(stackData, 1, 2, minObjSize, &ow);
  /*
  int nobj = Stack_Label_Large_Objects_N(
        stackData, NULL, 1, 2, minObjSize, 26);
        */
  //int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
  if (nobj == 0) {
    cout << "No object found in the image. No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }

  advanceProgress(0.1);

  if (nobj > 65533) {
    cout << "Too many objects ( > 65533). No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  size_t voxelNumber = C_Stack::voxelNumber(stackData);

  double step = 0.6 / nobj;
  for (int objIndex = 0; objIndex < nobj; objIndex++) {
    cout << "Skeletonizing object " << objIndex + 1 << "/" << nobj << endl;
    Swc_Tree *subtree = New_Swc_Tree();
    subtree->root = Make_Virtual_Swc_Tree_Node();

    Stack *objstack = Copy_Stack(stackData);
    size_t objSize = Stack_Level_Mask(objstack, 3 + objIndex);

    //Translate_Stack(objstack, GREY, 1);

    int objectOffset[3];
    Stack *croppedObjStack = C_Stack::boundCrop(objstack, 0, objectOffset);

    Stack *croppedSignal = NULL;
    if (stackSignal != NULL) {
      Cuboid_I bound_box;
      Stack_Bound_Box(objstack, &bound_box);
      croppedSignal = C_Stack::crop(stackSignal, bound_box, NULL);;
    }

    C_Stack::kill(objstack);
    objstack = NULL;

    /*
    if (C_Stack::kind(objstack) == GREY16) {
      Translate_Stack(objstack, GREY, 1);
    }
    */

    if (C_Stack::kind(croppedObjStack) == GREY16) {
      Translate_Stack(croppedObjStack, GREY, 1);
    }

    double linScale = 1.0;

    if (m_downsampleInterval[0] == m_downsampleInterval[1] &&
        m_downsampleInterval[1] == m_downsampleInterval[2]) {
      linScale = m_downsampleInterval[0] + 1;
    } else {
      linScale = Cube_Root(dsVol);
    }

    double lengthThreshold = m_lengthThreshold / linScale;
    if (objSize == 1) {
      if (m_keepingSingleObject || lengthThreshold <= 1) {
        int x = 0;
        int y = 0;
        int z = 0;
        for (size_t offset = 0; offset < voxelNumber; ++offset) {
          if (croppedObjStack->array[offset] == 1) {
            C_Stack::indexToCoord(offset, C_Stack::width(croppedObjStack),
                                  C_Stack::height(croppedObjStack), &x, &y, &z);
            break;
          }
        }
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              x + objectOffset[0], y + objectOffset[1],
            z + objectOffset[2], 1.0);
        SwcTreeNode::setParent(tn, subtree->root);
      }
    } else {
      cout << "Build distance map ..." << endl;
      Stack *tmpdist = Stack_Bwdist_L_U16P(croppedObjStack, NULL, 0);

      cout << "Shortest path grow ..." << endl;
      Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
      Sp_Grow_Workspace_Enable_Eucdist_Buffer(sgw);
      sgw->resolution[0] = m_resolution[0];
      sgw->resolution[1] = m_resolution[1];
      sgw->resolution[2] = m_resolution[2];
      sgw->wf = Stack_Voxel_Weight_I;

      size_t max_index;
      Stack_Max(tmpdist, &max_index);

      Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist),
                               Stack_Height(tmpdist),
                               Stack_Depth(tmpdist));
      Zero_Stack(mask);

      size_t nvoxel = Stack_Voxel_Number(croppedObjStack);
      size_t i;
      for (i = 0; i < nvoxel; i++) {
        if (croppedObjStack->array[i] == 0) {
          mask->array[i] = SP_GROW_BARRIER;
        }
      }

      mask->array[max_index] = SP_GROW_SOURCE;
      Sp_Grow_Workspace_Set_Mask(sgw, mask->array);

      if (croppedSignal != NULL) {
        //Stack_Add(croppedSignal, tmpdist, croppedSignal);
        Stack_Sp_Grow(croppedSignal, NULL, 0, NULL, 0, sgw);
      } else {
        Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
      }

      ZSpGrowParser parser(sgw);

      if (m_rebase) {
        cout << "Replacing start point ..." << endl;
        ZVoxelArray path = parser.extractLongestPath(NULL, false);
        for (i = 0; i < nvoxel; i++) {
          if (mask->array[i] != SP_GROW_BARRIER) {
            mask->array[i] = 0;
          }
        }

        ssize_t seedIndex = path.getInternalData()[0].toIndex(
              C_Stack::width(mask), C_Stack::height(mask),
              C_Stack::depth(mask));
        mask->array[seedIndex] = SP_GROW_SOURCE;

        if (croppedSignal != NULL) {
          Stack_Sp_Grow(croppedSignal, NULL, 0, NULL, 0, sgw);
        } else {
          Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
        }

        //Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
      }

      //double lengthThreshold = lengthThreshold;

      std::vector<ZVoxelArray> pathArray =
          parser.extractAllPath(lengthThreshold, tmpdist);

      if (pathArray.empty() && m_keepingSingleObject) {
        pathArray.push_back(parser.extractLongestPath(NULL, false));
      }

      //Make a subtree from a single object
      for (std::vector<ZVoxelArray>::iterator iter = pathArray.begin();
           iter != pathArray.end(); ++iter) {
        (*iter).sample(tmpdist, AdjustedDistanceWeight);
        //(*iter).labelStack(stackData, 255.0);

        ZSwcTree *branchWrapper =
            ZSwcGenerator::createSwc(*iter, ZSwcGenerator::REGION_SAMPLING);
        Swc_Tree *branch  = branchWrapper->data();
        branchWrapper->setData(branch, ZSwcTree::LEAVE_ALONE);
        branchWrapper->translate(objectOffset[0], objectOffset[1],
            objectOffset[2]);

        //branch = (*iter).toSwcTree();
        /********* Removel small temini ************/
        if (SwcTreeNode::firstChild(branch->root) != NULL) {
          if (SwcTreeNode::radius(branch->root) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::firstChild(branch->root))) {
            Swc_Tree_Node *oldRoot = branch->root;
            Swc_Tree_Node *newRoot = SwcTreeNode::firstChild(branch->root);
            SwcTreeNode::detachParent(newRoot);
            branch->root = newRoot;
            SwcTreeNode::kill(oldRoot);
          }
        }

        Swc_Tree_Node *leaf = SwcTreeNode::firstChild(branch->root);
        if (leaf != NULL) {
          while (SwcTreeNode::firstChild(leaf) != NULL) {
            leaf = SwcTreeNode::firstChild(leaf);
          }

          if (SwcTreeNode::radius(leaf) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::parent(leaf))) {
            SwcTreeNode::detachParent(leaf);
            SwcTreeNode::kill(leaf);
          }
        }
        /****************************/

        Swc_Tree_Node *tn = Swc_Tree_Connect_Branch(subtree, branch->root);

#ifdef _DEBUG_
        if (SwcTreeNode::length(branch->root) > 100) {
          std::cout << "Potential bug." << std::endl;
        }
#endif

        if (SwcTreeNode::isRegular(SwcTreeNode::parent(tn))) {
          if (SwcTreeNode::hasOverlap(tn, SwcTreeNode::parent(tn))) {
            SwcTreeNode::mergeToParent(tn);
          }
        }

        branch->root = NULL;
        Kill_Swc_Tree(branch);
        branch = NULL;

#ifdef _DEBUG_
        Swc_Tree_Iterator_Start(subtree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
        Swc_Tree_Node *tmptn = NULL;
        while ((tmptn = Swc_Tree_Next(subtree)) != NULL) {
          if (!SwcTreeNode::isRoot(tmptn)) {
            TZ_ASSERT(SwcTreeNode::length(tmptn) > 0.0, "duplicating nodes");
          }
        }

#endif
      }

      Kill_Stack(mask);
      Kill_Stack(tmpdist);
    }

    C_Stack::kill(croppedObjStack);
    croppedObjStack = NULL;
    C_Stack::kill(croppedSignal);
    croppedSignal = NULL;

    if (Swc_Tree_Regular_Root(subtree) != NULL) {
#ifdef _DEBUG_
      cout << Swc_Tree_Node_Fsize(subtree->root) - 1<< " nodes added" << endl;
#endif
      Swc_Tree_Merge(tree, subtree);
    }

    Kill_Swc_Tree(subtree);

    advanceProgress(step);
  }

  C_Stack::kill(stackData);
  stackData = NULL;
  C_Stack::kill(stackSignal);
  stackSignal = NULL;

  ZSwcTree *wholeTree = NULL;

  if (Swc_Tree_Regular_Root(tree) != NULL) {
    wholeTree = new ZSwcTree;
    wholeTree->setData(tree);

    if (m_downsampleInterval[0] > 0 || m_downsampleInterval[1] > 0 ||
        m_downsampleInterval[2] > 0) {
      wholeTree->rescale(m_downsampleInterval[0] + 1,
          m_downsampleInterval[1] + 1, m_downsampleInterval[2] + 1);
    }

    if (m_connectingBranch) {
      reconnect(wholeTree);
    }

    if (m_resampleSwc) {
      ZSwcResampler resampler;
      resampler.optimalDownsample(wholeTree);
    }

    wholeTree->resortId();
  }

  advanceProgress(0.05);
  endProgress();

  return wholeTree;
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const Stack *stack)
{
  Stack *stackData = Downsample_Stack_Max(stack, m_downsampleInterval[0],
      m_downsampleInterval[1], m_downsampleInterval[2], NULL);
  return makeSkeletonWithoutDs(stackData);
}

void ZStackSkeletonizer::reconnect(ZSwcTree *tree)
{
  if (tree->regularRootNumber() > 1) {
    double z_scale = m_resolution[2];
    if (m_resolution[0] != 1.0) {
      z_scale /= m_resolution[0];
    }
    Swc_Tree_Reconnect(tree->data(), z_scale,
                       m_distanceThreshold / m_resolution[0]);
    tree->resortId();
  }
}

void ZStackSkeletonizer::configure(const string &filePath)
{
  ZJsonObject obj;
  obj.load(filePath);
  configure(obj);
}

void ZStackSkeletonizer::configure(const ZJsonObject &config)
{
  ZJsonArray array(const_cast<json_t*>(config["downsampleInterval"]), false);
  std::vector<int> interval = array.toIntegerArray();
  if (interval.size() == 3) {
    setDownsampleInterval(interval[0], interval[1], interval[2]);
  } else {
    RECORD_WARNING_UNCOND("Invalid downsample parameter");
  }

  const json_t *value = config["minimalLength"];
  if (ZJsonParser::isNumber(value)) {
    setLengthThreshold(ZJsonParser::numberValue(value));
  }

  value = config["maximalDistance"];
  if (ZJsonParser::isNumber(value)) {
    setDistanceThreshold(ZJsonParser::numberValue(value));
  }

  value = config["keepingSingleObject"];
  if (ZJsonParser::isBoolean(value)) {
    setKeepingSingleObject(ZJsonParser::booleanValue(value));
  }

  value = config["rebase"];
  if (ZJsonParser::isBoolean(value)) {
    setRebase(ZJsonParser::booleanValue(value));
  }

  value = config["fillingHole"];
  if (ZJsonParser::isBoolean(value)) {
    setFillingHole(ZJsonParser::booleanValue(value));
  }

  value = config["minimalObjectSize"];
  if (ZJsonParser::isInteger(value)) {
    setMinObjSize(ZJsonParser::integerValue(value));
  }
}

void ZStackSkeletonizer::print() const
{
  std::cout << "Minimal length: " << m_lengthThreshold << std::endl;
  std::cout << "Maximal distance: " << m_distanceThreshold << std::endl;
  std::cout << "Rebase: " << m_rebase << std::endl;
  std::cout << "Intepolate: " << m_interpolating << std::endl;
  std::cout << "Remove border: " << m_removingBorder << std::endl;
  std::cout << "Filling hole: " << m_fillingHole << std::endl;
  std::cout << "Minimal object size: " << m_minObjSize << std::endl;
  std::cout << "Keep short object: " << m_keepingSingleObject << std::endl;
  std::cout << "Level: " << m_level << std::endl;
  std::cout << "Connect branch: " << m_connectingBranch << std::endl;
  std::cout << "Resolution: " << "(" << m_resolution[0] << ", "
            << m_resolution[1] << ", " << m_resolution[2] << ")" << std::endl;
  std::cout << "Downsample interval: (" << m_downsampleInterval[0] << ", "
            << m_downsampleInterval[1] << ", " << m_downsampleInterval[2]
            << ")" << std::endl;
}
