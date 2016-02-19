#ifndef ZDYNAMICPROGRAMMER_H
#define ZDYNAMICPROGRAMMER_H

#include <vector>
#include <map>
#include <utility>

class ZMatrix;

class ZDynamicProgrammer
{
public:
  ZDynamicProgrammer();

  class MatchResult : public std::vector<std::pair<int, int> > {
  };

  double match(const ZMatrix &simmat);

  inline void setGapPenalty(double penalty) {
    m_gapPenalty = penalty;
  }

  inline const MatchResult& getMatchingResult() const {
    return m_matchingResult; }

  inline double getScore() const { return m_matchingScore; }

private:
  double m_gapPenalty;
  MatchResult m_matchingResult;
  double m_matchingScore;
};

#endif // ZDYNAMICPROGRAMMER_H
