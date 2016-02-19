#include "zswcglobalfeatureanalyzer.h"
#include "zswctreenodearray.h"
#include "swctreenode.h"
#include "zswcdisttrunkanalyzer.h"
#include "zvectorgenerator.h"
#include "zpointarray.h"
#include "zeigensolver.h"
#include "zdoublevector.h"


#define GENERATE_NGF1_FEATURE \
  ZVectorGenerator<std::string>() << "Number of leaves"\
                                << "number of branch points"\
                                << "box volume"\
                                << "maximum segment length"\
                                << "maximum path length"\
                                << "average radius"\
                                << "radius variance"\
                                << "lateral/vertical ratio"\
                                << "Average curvature"

std::vector<std::string> ZSwcGlobalFeatureAnalyzer::m_ngf1FeatureName =
    GENERATE_NGF1_FEATURE;

std::vector<std::string> ZSwcGlobalFeatureAnalyzer::m_ngf3FeatureName =
    GENERATE_NGF1_FEATURE
    << "laberal span"
    << "vertical span";

std::string ZSwcGlobalFeatureAnalyzer::m_emptyFeatureName = "";

ZSwcGlobalFeatureAnalyzer::ZSwcGlobalFeatureAnalyzer()
{
}

double ZSwcGlobalFeatureAnalyzer::computeBoxLateralVerticalRatio(
    const ZSwcTree &tree)
{
  ZCuboid box =tree.getBoundBox();

  return sqrt(box.width() * box.width() + box.height() * box.height()) /
      box.depth();
}

double ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(
    const ZSwcTree &tree)
{
  return computeLateralSpan(tree) / computeVerticalSpan(tree);
}

int ZSwcGlobalFeatureAnalyzer::getFeatureNumber(EFeatureSet setName)
{
  switch (setName) {
  case NGF1:
    return m_ngf1FeatureName.size();
  case NGF3:
    return m_ngf3FeatureName.size();
  default:
    return 0;
  }

  return 0;
}

//Number of leaves, number of branch points,
//box volume, maximum segment length, maximum path length
//average radius
std::vector<double> ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
    ZSwcTree &tree, EFeatureSet setName)
{
  std::vector<double> featureSet;
  switch (setName) {
  case NGF1:
  {
    featureSet.resize(9, 0.0);
    if (tree.isEmpty()) {
      break;
    }
    int leafCount = 0;
    int branchPointCount = 0;
    double boxVolume = 0;
    double averageRadius = 0;
    double radiusVariance = 0;
    double count = 0;
    double maxPathLength = 0.0;
    double maxSegmentLength = 0.0;
    double averageCurvature = 0.0;
    int curvatureCount = 0;

    ZSwcTreeNodeArray nodeArray = tree.getSwcTreeNodeArray();
    for (ZSwcTreeNodeArray::const_iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;
      if (SwcTreeNode::isRegular(tn)) {
        if (SwcTreeNode::isLeaf(tn)) {
          ++leafCount;
        }
        if (SwcTreeNode::isBranchPoint(tn)) {
          ++branchPointCount;
        }
        if (SwcTreeNode::isTerminal(tn)) {
          //Check root branch point
        }
        averageRadius += SwcTreeNode::radius(tn);
        radiusVariance += SwcTreeNode::radius(tn) * SwcTreeNode::radius(tn);
        ++count;

        if (SwcTreeNode::isContinuation(tn)) {
          double a = SwcTreeNode::distance(tn, SwcTreeNode::parent(tn));
          double b = SwcTreeNode::distance(tn, SwcTreeNode::firstChild(tn));
          double c = SwcTreeNode::distance(
                SwcTreeNode::parent(tn), SwcTreeNode::firstChild(tn));
          if (a > 0.0 && b > 0.0 && c > 0.0) {
            ++curvatureCount;
            double u = b + c - a;
            double v = a + c - b;
            double w = a + b - c;
            if (u > 0.0 && v > 0.0 && w > 0.0) {
              averageCurvature +=
                  sqrt(a + b + c) / a * sqrt(u) / b * sqrt(v) / c * sqrt(w);
            }
          }
        }
      }
    }
    averageRadius /= count;
    radiusVariance = radiusVariance / count - averageRadius * averageRadius;

    ZCuboid box = tree.getBoundBox();
    boxVolume = box.volume();

    ZSwcDistTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.setDistanceWeight(0.0, 1.0);
    ZSwcPath path = trunkAnalyzer.extractMainTrunk(&tree);
    maxPathLength = path.getLength();

    maxSegmentLength = tree.getMaxSegmentLenth();

    double lvRatio = sqrt(box.width() * box.width() + box.height() * box.height()) /
        box.depth();

    if (curvatureCount > 0) {
      averageCurvature /= curvatureCount;
    }

    featureSet[0] = leafCount;
    featureSet[1] = branchPointCount;
    featureSet[2] = boxVolume;
    featureSet[3] = maxSegmentLength;
    featureSet[4] = maxPathLength;
    featureSet[5] = averageRadius;
    featureSet[6] = radiusVariance;
    featureSet[7] = lvRatio;
    featureSet[8] = averageCurvature;
  }
    break;
  case NGF3:
    featureSet = computeFeatureSet(tree, NGF1);
    featureSet.push_back(computeLateralSpan(tree));
    featureSet.push_back(computeVerticalSpan(tree));
    break;
  default:
    break;
  }

  return featureSet;
}

std::vector<std::string>
ZSwcGlobalFeatureAnalyzer::getFeatureNameArray(EFeatureSet setName)
{
  std::vector<std::string> nameArray;
  switch(setName) {
  case NGF1:
    nameArray = m_ngf1FeatureName;
    break;
  case NGF3:
    nameArray = m_ngf3FeatureName;
    break;
  default:
    break;
  }

  return nameArray;
}

const std::string& ZSwcGlobalFeatureAnalyzer::getFeatureName(
    EFeatureSet setName, int index)
{
  if (index < 0) {
    return m_emptyFeatureName;
  }

  switch (setName) {
  case NGF1:
    if (index >= (int) m_ngf1FeatureName.size()) {
      return m_emptyFeatureName;
    } else {
      return m_ngf1FeatureName[index];
    }
    break;
  case NGF3:
    if (index >= (int) m_ngf1FeatureName.size()) {
      return m_emptyFeatureName;
    } else {
      return m_ngf3FeatureName[index];
    }
    break;
  default:
    break;
  }

  return m_emptyFeatureName;
}

double ZSwcGlobalFeatureAnalyzer::computeLateralSpan(const ZSwcTree &tree)
{
  ZSwcTree::DepthFirstIterator treeIter(&tree);
  ZPointArray ptArray;
  for (Swc_Tree_Node *tn = treeIter.begin(); tn != NULL;
       tn = treeIter.next()) {
    if (SwcTreeNode::isRegular(tn)) {
      ptArray.append(SwcTreeNode::center(tn));
    }
  }

  std::vector<double> cov = ptArray.computePlaneCov();
  ZEigenSolver solver;
  solver.solveCovEigen(cov);
  double span = sqrt(solver.getEigenValue(0));

  return span;
}

double ZSwcGlobalFeatureAnalyzer::computeVerticalSpan(const ZSwcTree &tree)
{
  ZDoubleVector zArray;
  ZSwcTree::DepthFirstIterator treeIter(&tree);
  for (Swc_Tree_Node *tn = treeIter.begin(); tn != NULL;
       tn = treeIter.next()) {
    zArray.push_back(SwcTreeNode::z(tn));
  }

#ifdef _DEBUG_2
  std::vector<int> indexArray;
  zArray.sort(indexArray);
  zArray.print();
#endif

  return sqrt(zArray.var());
}
