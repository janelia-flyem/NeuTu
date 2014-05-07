#ifndef ZDVIDFILTER_H
#define ZDVIDFILTER_H

#include "dvid/zdvidtarget.h"
#include <set>
#include <vector>

/*!
 * \brief The class of filtering data from dvid
 */
class ZDvidFilter
{
public:
  ZDvidFilter();

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  inline size_t getMinBodySize() const {
    return m_minBodySize;
  }

  inline size_t getMaxBodySize() const {
    return m_maxBodySize;
  }

  inline void setMinBodySize(size_t s) {
    m_minBodySize = s;
  }

  inline void setMaxBodySize(size_t s) {
    m_maxBodySize = s;
  }

  void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  void exclude(int bodyId);
  void exclude(const std::vector<int> &bodyArray);
  bool isExcluded(int bodyId) const;

private:
  ZDvidTarget m_dvidTarget;
  size_t m_minBodySize;
  size_t m_maxBodySize;

  std::set<int> m_excludedBodySet;
};

#endif // ZDVIDFILTER_H
