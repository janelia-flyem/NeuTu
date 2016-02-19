#ifndef ZSTACKBINARIZER_H
#define ZSTACKBINARIZER_H

#include "c_stack.h"

/*!
 * \brief The class for binarizing a stack
 *
 * The input stack must be GREY or GREY16 kind.
 */
class ZStackBinarizer
{
public:
  ZStackBinarizer();

  enum EMethod {
    BM_MANUAL, BM_RC_THRESHOLD, BM_STABLE_POINT, BM_TRIANGLE, BM_LOCMAX,
    BM_MEAN, BM_ONE_SIGMA, BM_NSIGMA
  };

  bool binarize(Stack *stack);

  inline void setMethod(EMethod method) {
    m_method = method;
  }

  inline void setThreshold(int threshold) {
    m_threshold = threshold;
  }

  inline void setLowerBound(int lower) {
    m_lowerBound = lower;
  }

  inline void setUpperBound(int upper) {
    m_upperBound = upper;
  }

  inline void setRetryCount(int count) {
    m_retryCount = count;
  }

  inline void setMinObjectSize(int s) {
    m_minObjectSize = s;
  }

private:
  void binarizeByLocmax(Stack *stack);
  int *computeLocmaxHist(const Stack *stack);
  int refineLocmaxThreshold(const Stack *stack, int threshold, int *hist,
                               int upperBound);
  void postProcess(Stack *stack);

private:
  Stack *m_reference; //reference stack
  EMethod m_method;
  int m_threshold;
  int m_lowerBound;
  int m_upperBound;
  int m_retryCount;
  int m_minObjectSize;
  double m_sigmaScale;
};

#endif // ZSTACKBINARIZER_H
