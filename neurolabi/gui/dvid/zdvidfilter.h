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
    m_hasUpperBodySize = true;
    m_maxBodySize = s;
  }

  inline void setUpperBodySizeEnabled(bool enabled) {
    m_hasUpperBodySize = enabled;
  }

  inline bool hasUpperBodySize() const {
    return m_hasUpperBodySize;
  }

  void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  void exclude(int bodyId);
  void exclude(const std::vector<int> &bodyArray);
  bool isExcluded(int bodyId) const;
  bool hasExclusion() const;

private:
  ZDvidTarget m_dvidTarget;
  size_t m_minBodySize;
  size_t m_maxBodySize;
  bool m_hasUpperBodySize;

  std::set<int> m_excludedBodySet;
};

#endif // ZDVIDFILTER_H
