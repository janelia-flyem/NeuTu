#include "zdynamicprogrammer.h"
#include "zmatrix.h"
#include "zinttree.h"

ZDynamicProgrammer::ZDynamicProgrammer()
{
}

double ZDynamicProgrammer::match(const ZMatrix &simMat)
{
  ZMatrix matchingTable(
        simMat.getRowNumber() + 1, simMat.getColumnNumber() + 1);
  matchingTable.setConstant(0.0);

  ZIntTree matchingTree(simMat.getSize());

  for (int i = 0; i < simMat.getRowNumber(); i++) {
    for (int j = 0; j < simMat.getColumnNumber(); j++) {
      double score = matchingTable.getValue(i, j) + simMat.getValue(i, j);
      double maxScore = score;
      matchingTree.setParent(simMat.sub2index(i, j),
                            simMat.sub2index(i - 1, j - 1));
      score = matchingTable.getValue(i + 1, j) - m_gapPenalty;
      if (score > maxScore) {
        maxScore = score;
        matchingTree.setParent(simMat.sub2index(i, j),
                              simMat.sub2index(i, j - 1));
      }
      score = matchingTable.getValue(i, j + 1) - m_gapPenalty;
      if (score > maxScore) {
        maxScore = score;
        matchingTree.setParent(simMat.sub2index(i, j),
                              simMat.sub2index(i - 1, j));
      }
      matchingTable.set(i + 1, j + 1, maxScore);
    }
  }

  int bestIndex = -1;
  m_matchingScore = -m_gapPenalty * 10.0; //Set to a small value
  int lastColumnIndex = matchingTable.getColumnNumber() - 1;
  int lastRowIndex = matchingTable.getRowNumber() - 1;
  for (int i = 1; i < matchingTable.getRowNumber(); i++) {
    if (matchingTable.getValue(i, lastColumnIndex) > m_matchingScore) {
      m_matchingScore = matchingTable.getValue(i, lastColumnIndex);
      bestIndex = simMat.sub2index(i - 1, lastColumnIndex - 1);
    }
  }
  for (int i = 1; i < matchingTable.getColumnNumber(); i++) {
    if (matchingTable.getValue(lastRowIndex, i) > m_matchingScore) {
      m_matchingScore = matchingTable.getValue(lastRowIndex, i);
      bestIndex = simMat.sub2index(lastRowIndex - 1, i - 1);
    }
  }

#ifdef _DEBUG_2
  matchingTable.debugOutput();
  std::cout << std::endl;
#endif

  std::vector<int> matchingTrace = matchingTree.traceBack(bestIndex);

  for (size_t i = 0; i < matchingTrace.size(); i++) {
    std::pair<int, int> sub = simMat.index2sub(matchingTrace[i]);
    m_matchingResult.push_back(std::pair<int, int>(sub.first, sub.second));
#ifdef _DEBUG_2
    std::cout << "s: " << simMat.getValue(sub.first, sub.second)
              << " " << matchingTable.getValue(sub.first + 1, sub.second + 1)
              << std::endl;
#endif
  }

  return m_matchingScore;
}
