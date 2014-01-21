/**@file skeletonize.cpp
 * @author Ting Zhao
 */
#include <iostream>
#include <sstream>
#include <fstream>

#include <string.h>
#include <stdlib.h>

#include "tz_utilities.h"
#include "zsegmentmaparray.h"
#include "zsuperpixelmaparray.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_stack_document.h"
#include "tz_xml_utils.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_objlabel.h"
#include "tz_swc_tree.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "tz_sp_grow.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "tz_stack_lib.h"
#include "tz_stack_stat.h"
#include "zstackskeletonizer.h"
#include "c_stack.h"
#include "zobject3d.h"
#include "zobject3dscan.h"
#include "zfiletype.h"
#include "zstring.h"
#include "tz_constant.h"
#include "zargumentprocessor.h"
#include "tz_math.h"
#include "swc/zswcresampler.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.3") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
                               "[--intv <int> <int> <int>]",
                               "[--minlen <int(15)>] [--maxdist <int(50)>]",
                               "[--minobj <int(0)>]",
                               "[--keep_short] [--save_offset]",
                               "[--interpolate] [--rmborder]",
                               "[--rebase]", "[--level <int>]",
                               "[--fill_hole]",
                               NULL};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  const char *input = ZArgumentProcessor::getStringArg("input");

  /* Skeletonization */
  cout << "Read stack ...\n" << endl;

  Stack *stack = NULL;
  int offset[3] = { 0, 0, 0 };

  int dsIntv[3] = { 0, 0, 0 };
  if (ZArgumentProcessor::isArgMatched("--intv")) {
    for (int i = 0; i < 3; ++i) {
      dsIntv[i] = ZArgumentProcessor::getIntArg("--intv", i + 1);
    }
  }

  bool isDownsample = (dsIntv[0] > 0 || dsIntv[1] > 0 || dsIntv[2] > 0);


  bool isBinarized = false;

  if (ZFileType::fileType(input) == ZFileType::OBJECT_SCAN_FILE) {
    ZObject3dScan obj;
    if (obj.load(input)) {
      if (isDownsample) {
        cout << "Downsampling ..." << endl;
        obj.downsampleMax(dsIntv[0], dsIntv[1], dsIntv[2]);
      }
      stack = obj.toStack(offset);
      isBinarized = true;

      if (isDownsample) {
        for (int i = 0; i < 3; ++i) {
          offset[i] *= dsIntv[i] + 1;
        }
      }
    }
  } else {
    stack = C_Stack::readSc(input);

    if (isDownsample) {
      cout << "Downsampling ..." << endl;
      Stack *bufferStack = Downsample_Stack_Max(
            stack, dsIntv[0], dsIntv[1], dsIntv[2], NULL);

      C_Stack::kill(stack);
      stack = bufferStack;
    }
  }

  if (stack == NULL) {
    cerr << "Failed to load stack data: " << input << endl;
    cerr << "Abort." << endl;
  }

  size_t voxelNumber = C_Stack::voxelNumber(stack);
  while (voxelNumber > MAX_INT32) {
    std::cout << "Stack too big. Downsampling triggered automatically." << std::endl;
    std::cout << "Downsampling ..." << std::endl;
    Stack *bufferStack = Downsample_Stack_Max(
          stack, 1, 1, 1, NULL);
    C_Stack::kill(stack);
    stack = bufferStack;

    for (int i = 0; i < 3; ++i) {
      dsIntv[i] = dsIntv[i] * 2 + 1;
    }
    isDownsample = true;

    voxelNumber = C_Stack::voxelNumber(stack);
  }

  if (!isBinarized) {
    cout << "Binarizing ..." << endl;
    if (ZArgumentProcessor::isArgMatched("--level")) {
      Stack_Binarize_Level(stack, ZArgumentProcessor::getIntArg("--level"));
      if (C_Stack::kind(stack) == GREY16) {
        Translate_Stack(stack, GREY, 1);
      }
    } else {
      Stack_Binarize(stack);
    }
  }

  if (Stack_Max(stack, NULL) != 1) {
    cout << "The stack might contain no object. Abort" << endl;
    return 1;
  }

  if (ZArgumentProcessor::isArgMatched("--rmborder")) {
    printf("Remove borders ...\n");
    Stack_Not(stack, stack);
    Stack* solid = Stack_Majority_Filter(stack, NULL, 8);
    C_Stack::kill(stack);

    Stack_Not(solid, solid);
    stack = solid;
  }

  if (ZArgumentProcessor::isArgMatched("--interpolate")) {
    printf("Interpolating ...\n");
    Stack *bufferStack = Stack_Bwinterp(stack, NULL);
    C_Stack::kill(stack);
    stack = bufferStack;
  }

  if (ZArgumentProcessor::isArgMatched("--fill_hole")) {
    std::cout << "Filling hole ..." << std::endl;
    Stack *bufferStack = Stack_Fill_Hole_N(stack, NULL, 1, 26, NULL);
    C_Stack::kill(stack);
    stack = bufferStack;
  }

  ZStackSkeletonizer skeletonizer;
  if (ZArgumentProcessor::isArgMatched("--rebase")) {
    skeletonizer.setRebase(true);
  } else {
    skeletonizer.setRebase(false);
  }

  int minObjSize = ZArgumentProcessor::getIntArg("--minobj");
  int maxDist = ZArgumentProcessor::getIntArg("--maxdist");
  int minLen = ZArgumentProcessor::getIntArg("--minlen");

  if (isDownsample) {
    int dsVol = (dsIntv[0] + 1) * (dsIntv[1] + 1) * (dsIntv[2] + 1);
    minObjSize /= dsVol;
    double linScale = 1.0;

    if (dsIntv[0] == dsIntv[1] && dsIntv[1] == dsIntv[2]) {
      linScale = dsIntv[0] + 1;
    } else {
      linScale = Cube_Root(dsVol);
    }
    maxDist /= linScale;
    minLen /= linScale;
  }

  skeletonizer.setMinObjSize(minObjSize);
  skeletonizer.setDistanceThreshold(maxDist);
  skeletonizer.setLengthThreshold(minLen);
  if (ZArgumentProcessor::isArgMatched("--keep_short")) {
    skeletonizer.setKeepingSingleObject(true);
  } else {
    skeletonizer.setKeepingSingleObject(false);
  }

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(stack);

  if (wholeTree != NULL) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);
    if (isDownsample) {
      wholeTree->rescale(dsIntv[0] + 1, dsIntv[1] + 1, dsIntv[2] + 1);
    }

    ZString outputPath = ZArgumentProcessor::getStringArg("-o");
    if (ZArgumentProcessor::isArgMatched("--save_offset")) {
      outputPath.replace(".swc", ".offset.txt");
      ofstream stream(outputPath.c_str());
      stream << offset[0] << " " << offset[1] << " " << offset[2];
      stream.close();
    } else {
      wholeTree->translate(offset[0], offset[1], offset[2]);
    }

    wholeTree->save(outputPath.c_str());
    cout << outputPath << " saved" << endl;
  } else {
    std::cout << "Empty tree. No file saved";
    return 1;
  }

  return 0;
}
