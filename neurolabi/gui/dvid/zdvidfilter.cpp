#include "zdvidfilter.h"
#include <iostream>

ZDvidFilter::ZDvidFilter()
{
}

void ZDvidFilter::exclude(int bodyId)
{
  m_excludedBodySet.insert(bodyId);
}

void ZDvidFilter::exclude(const std::vector<int> &bodyArray)
{
  for (std::vector<int>::const_iterator iter = bodyArray.begin();
       iter != bodyArray.end(); ++iter) {
    exclude(*iter);
  }
}

bool ZDvidFilter::isExcluded(int bodyId) const
{
#ifdef _DEBUG_
  if (bodyId == 16493) {
    std::cout << "16493: " << m_excludedBodySet.count(bodyId) << std::endl;
  }
#endif
  return m_excludedBodySet.count(bodyId) > 0;
}
