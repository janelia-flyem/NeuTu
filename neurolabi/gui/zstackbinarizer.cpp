#include "zstackbinarizer.h"
#include <iostream>
#include <cmath>
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "c_stack.h"
#include "tz_stack_lib.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_stat.h"
#include "tz_stack_bwmorph.h"
#include "tz_math.h"

ZStackBinarizer::ZStackBinarizer() : m_reference(NULL), m_method(BM_MANUAL),
  m_threshold(0), m_lowerBound(-1), m_upperBound(-1), m_retryCount(0),
  m_minObjectSize(0), m_sigmaScale(1.0)
{
}

#define BINARIZE_CLEAN \
  if (refStack != stack) { \
    C_Stack::kill(refStack); \
  } \
  free(hist);

bool ZStackBinarizer::binarize(Stack *stack)
{
  if (stack == NULL) {
    return false;
  }

  int threshold = m_threshold;
  int *hist = NULL;

  Stack *refStack = C_Stack::clone(m_reference);
  if (refStack == NULL) {
    refStack = stack;
  }

  int low = m_lowerBound;
  int high = m_upperBound;

  switch (m_method) {
  case BM_RC_THRESHOLD:
  case BM_STABLE_POINT:
  case BM_TRIANGLE:
    hist = Stack_Hist(refStack);
    break;
  case BM_LOCMAX:
    hist = computeLocmaxHist(refStack);
    break;
  default:
    break;
  }

  if (hist != NULL) {
    if (m_lowerBound < 0) {
      low = Int_Histogram_Min(hist);
    }

    if (m_upperBound < 0) {
      high = Int_Histogram_Max(hist);
    }
  }

  bool succ = true;

  if (low == high && hist != NULL) {
    succ = false;
  }


  if (succ) {
    switch (m_method) {
    case BM_RC_THRESHOLD:
      threshold = Int_Histogram_Rc_Threshold(hist, low, high);
      break;
    case BM_STABLE_POINT:
      threshold = Int_Histogram_Stable_Point(hist, low, high);
      break;
    case BM_TRIANGLE:
      threshold = Int_Histogram_Triangle_Threshold(hist, low, high);
      break;
    case BM_LOCMAX:
      if ((double) Int_Histogram_Sum(hist) / C_Stack::voxelNumber(stack) <= 1e-5) {
        threshold = Stack_Common_Intensity(stack, 0, high);
      } else {
        threshold = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
        threshold = refineLocmaxThreshold(refStack, threshold, hist, high);
      }
      /*
    if (threshold <= Int_Histogram_Quantile(hist, 0.5)) {
      BINARIZE_CLEAN
      return false;
    }
    */
      break;
    case BM_MEAN:
      threshold = iround(Stack_Mean(stack));
      break;
    case BM_ONE_SIGMA:
      threshold = iround(Stack_Mean(stack) + sqrt(Stack_Var(stack)));
      break;
    case BM_NSIGMA:
      threshold =
          iround(Stack_Mean(stack) + m_sigmaScale * sqrt(Stack_Var(stack)));
      break;
    default:
      break;
    }

#ifdef _DEBUG_
    std::cout << "Threshold: " << threshold << std::endl;
#endif

    Stack_Threshold_Binarize(stack, threshold);
  } else {
    Stack_Binarize(stack);
  }

  BINARIZE_CLEAN

  postProcess(stack);


  return succ;
}

int* ZStackBinarizer::computeLocmaxHist(const Stack *stack)
{
  int conn = 18;
  Stack *locmax = Stack_Locmax_Region(stack, conn);
  Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
  int nvoxel = Stack_Voxel_Number(locmax);
  int i;

  for (i = 0; i < nvoxel; i++) {
    if (locmax->array[i] < 3) {
      locmax->array[i] = 0;
    } else {
      locmax->array[i] = 1;
    }
  }
  int *hist = Stack_Hist_M(stack, locmax);

  return hist;
}

int ZStackBinarizer::refineLocmaxThreshold(
    const Stack *stack, int thre, int *hist, int upperBound)
{
  const double ratio_low_thre = 0.01;
  const double ratio_thre = 0.05;
  int thre2;

  double fgratio = (double) Stack_Fgarea_T(stack, thre) /
      Stack_Voxel_Number(stack);

  double fgratio2 = fgratio;
  double prev_fgratio = fgratio;

  if ((fgratio > ratio_low_thre) && (fgratio <= ratio_thre)) {
    thre2 = Int_Histogram_Triangle_Threshold(hist, thre + 1, upperBound - 1);
    fgratio2 = (double) Stack_Fgarea_T(stack, thre2) /
        Stack_Voxel_Number(stack);
    if (fgratio2 / fgratio <= 0.3) {
      thre = thre2;
    }
  } else {
    int nretry = m_retryCount;
    thre2 = thre;
    while (fgratio2 > ratio_thre) {
      thre2 = Int_Histogram_Triangle_Threshold(hist, thre2 + 1, upperBound - 1);
      fgratio2 = (double) Stack_Fgarea_T(stack, thre2) /
          Stack_Voxel_Number(stack);
      if (fgratio2 / prev_fgratio <= 0.5) {
        thre = thre2;
      }
      prev_fgratio = fgratio2;

      nretry--;

      if (nretry == 0) {
        break;
      }
    }
  }

  return thre;
}

void ZStackBinarizer::postProcess(Stack *stack)
{
  if (C_Stack::kind(stack) != GREY) {
    Translate_Stack(stack, GREY, 1);
  }

  if (m_minObjectSize > 0) {
    Stack *tmpStack = C_Stack::clone(stack);
    stack = Stack_Remove_Small_Object(tmpStack, stack, m_minObjectSize, 26);
    C_Stack::kill(tmpStack);

    if (C_Stack::kind(stack) != GREY) {
      Translate_Stack(stack, GREY, 1);
    }
  }

  if (stack->kind != GREY) {
    Translate_Stack(stack, GREY, 1);
  }
}
