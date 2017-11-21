#ifndef ZSWCDIRECTIONFEATUREANALYZER_H
#define ZSWCDIRECTIONFEATUREANALYZER_H

#include "zswcfeatureanalyzer.h"

class ZSwcDirectionFeatureAnalyzer : public ZSwcFeatureAnalyzer
{
public:
  ZSwcDirectionFeatureAnalyzer();

 std::vector<double> computeFeature(Swc_Tree_Node *tn);
  double computeFeatureSimilarity(
      const std::vector<double> &featureArray1,
      const std::vector<double> &featureArray2);

  void setParameter(const std::vector<double> &parameterArray);
};

#endif // ZSWCDIRECTIONFEATUREANALYZER_H
