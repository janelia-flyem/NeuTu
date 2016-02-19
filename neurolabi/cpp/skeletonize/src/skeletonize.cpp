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
#include "zjsonparser.h"

using namespace std;

static int help(int argc, char *argv[], const char *spec[])
{
  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0) {
      printf("\nskeletonize ");
      Print_Argument_Spec(spec);

      printf("\nDetails\n\n");
      printf("input: Input image file.\n\n");
      printf("-o: Output skeleton file (swc).\n\n");
      printf("--intv: Downsample interval for intermediate process. "
             "The final skeleton is always in the orignal scale.\n\n");
      printf("--minlen: Minimal length of a branch. This is not an absolute "
             "contraint, i.e. the final skeleton may still have a branch "
             "shorter than this number. \n\n");
      printf("--maxdist: Maximum distance of a gap to be connected "
             "(adjusted to the original scale in the case of downsample).\n\n");
      printf("--keep_short: Keep isolated object even if it is shorter or "
             "smaller than the threshold.\n\n");
      printf("--save_offset (Obsolete): Save the offset to the orignal stack.\n\n");
      printf("--interpolate: Slice-wise interpolation of the image.\n\n");
      printf("--rmborder: Remove border of boder caused by the segmentation "
             "pipeline. Ignore it if you don't know what it is.\n\n");
      printf("--rebase: Reset the start point to presever the longest branch.\n\n");
      printf("--level: Gray scale of the object to skeletonize.\n\n");
      printf("--fill_hole: Fill the hole of the object before skeletonization.\n\n");

      return 1;
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.5") == 1) {
    return 0;
  }

  static char const *Spec[] = {"<input:string> [-o <string>]",
                               "[--intv <int> <int> <int>]",
                               "[--minlen <int(15)>] [--maxdist <int(50)>]",
                               "[--minobj <int(0)>]",
                               "[--keep_short]",
                               "[--interpolate] [--rmborder]",
                               "[--rebase]", "[--level <int>]",
                               "[--fill_hole]",
                               "[--config <string>]",
                               NULL};

  if (help(argc, argv, Spec) == 1) {
    return 0;
  }

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  const char *input = ZArgumentProcessor::getStringArg("input");


  ZStackSkeletonizer skeletonizer;
  if (ZArgumentProcessor::isArgMatched("--config")) {
    skeletonizer.configure(ZArgumentProcessor::getStringArg("--config"));
  }

  /* Skeletonization */
  cout << "Read stack ...\n" << endl;

  Stack *stack = NULL;
  //int offset[3] = { 0, 0, 0 };

  int dsIntv[3] = { 0, 0, 0 };
  if (ZArgumentProcessor::isArgMatched("--intv")) {
    for (int i = 0; i < 3; ++i) {
      dsIntv[i] = ZArgumentProcessor::getIntArg("--intv", i + 1);
    }
  } else {
    skeletonizer.getDownsampleInterval(dsIntv, dsIntv + 1, dsIntv + 2);
  }

  bool isBinarized = false;


  ZObject3dScan obj;
  if (ZFileType::fileType(input) == ZFileType::OBJECT_SCAN_FILE) {
    obj.load(input);
    /*
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
    */
  } else {
    stack = C_Stack::readSc(input);

    /*
    if (isDownsample) {
      cout << "Downsampling ..." << endl;
      Stack *bufferStack = Downsample_Stack_Max(
            stack, dsIntv[0], dsIntv[1], dsIntv[2], NULL);

      C_Stack::kill(stack);
      stack = bufferStack;
    }
    */
  }

  if (stack == NULL && obj.isEmpty()) {
    cerr << "Failed to load stack data: " << input << endl;
    cerr << "Abort." << endl;
    
    return 1;
  }

  if (stack != NULL) {
    size_t voxelNumber = C_Stack::voxelNumber(stack);
    voxelNumber /= (dsIntv[0] + 1) * (dsIntv[1] + 1) * (dsIntv[2] + 1);
    while (voxelNumber > MAX_INT32) {
      std::cout << "Stack too big. Downsampling triggered automatically." << std::endl;

      for (int i = 0; i < 3; ++i) {
        dsIntv[i] = dsIntv[i] * 2 + 1;
      }

      voxelNumber /= (dsIntv[0] + 1) * (dsIntv[1] + 1) * (dsIntv[2] + 1);
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
  }

  skeletonizer.setDownsampleInterval(dsIntv[0], dsIntv[1], dsIntv[2]);
  if (ZArgumentProcessor::isArgMatched("--rmborder")) {
    skeletonizer.setRemovingBorder(true);
  }
  if (ZArgumentProcessor::isArgMatched("--interpolate")) {
    skeletonizer.setInterpolating(true);
  }
  if (ZArgumentProcessor::isArgMatched("--fill_hole")) {
    skeletonizer.setFillingHole(true);
  }
  if (ZArgumentProcessor::isArgMatched("--rebase")) {
    skeletonizer.setRebase(true);
  }
  if (ZArgumentProcessor::isArgMatched("--keep_short")) {
    skeletonizer.setKeepingSingleObject(true);
  }

  int minObjSize = ZArgumentProcessor::getIntArg("--minobj");
  int maxDist = ZArgumentProcessor::getIntArg("--maxdist");
  int minLen = ZArgumentProcessor::getIntArg("--minlen");

  skeletonizer.setMinObjSize(minObjSize);
  skeletonizer.setDistanceThreshold(maxDist);
  skeletonizer.setLengthThreshold(minLen);

  skeletonizer.print();

  /*
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
  */

  ZSwcTree *wholeTree = NULL;
  
  if (stack != NULL) {
    wholeTree = skeletonizer.makeSkeleton(stack);
  } else {
    wholeTree = skeletonizer.makeSkeleton(obj);
  }

  if (wholeTree != NULL) {
/*
    ZSwcResampler resampler;
    resampler.optimalDownsample(wholeTree);
    if (isDownsample) {
      wholeTree->rescale(dsIntv[0] + 1, dsIntv[1] + 1, dsIntv[2] + 1);
    }
    */

    ZString outputPath = ZArgumentProcessor::getStringArg("-o");
    /*
    if (ZArgumentProcessor::isArgMatched("--save_offset")) {
      outputPath.replace(".swc", ".offset.txt");
      ofstream stream(outputPath.c_str());
      stream << offset[0] << " " << offset[1] << " " << offset[2];
      stream.close();
    } else {
      wholeTree->translate(offset[0], offset[1], offset[2]);
    }
    */

    wholeTree->save(outputPath.c_str());
    cout << outputPath << " saved" << endl;
  } else {
    std::cout << "Empty tree. No file saved";
    return 1;
  }

  return 0;
}
