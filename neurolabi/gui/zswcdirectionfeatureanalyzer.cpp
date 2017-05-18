#include "zswcdirectionfeatureanalyzer.h"

#include "swctreenode.h"

ZSwcDirectionFeatureAnalyzer::ZSwcDirectionFeatureAnalyzer()
{

}

std::vector<double> ZSwcDirectionFeatureAnalyzer::computeFeature(Swc_Tree_Node *tn)
{
  std::vector<double> result;

  result.push_back(SwcTreeNode::x(tn));
  result.push_back(SwcTreeNode::y(tn));
  result.push_back(SwcTreeNode::z(tn));

  int traceBackCount = 5;
  ZPoint direction = SwcTreeNode::upStreamDirection(tn, traceBackCount);

  result.push_back(direction.getX());
  result.push_back(direction.getY());
  result.push_back(direction.getZ());

  return result;
}

double ZSwcDirectionFeatureAnalyzer::computeFeatureSimilarity(
    const std::vector<double> &/*featureArray1*/,
    const std::vector<double> &/*featureArray2*/)
{
  return 0.0;
}

void ZSwcDirectionFeatureAnalyzer::setParameter(
    const std::vector<double> &/*parameterArray*/)
{

}
