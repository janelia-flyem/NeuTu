#include "zdvidfilter.h"
#include <iostream>
#include "zstring.h"
#include "zfiletype.h"

ZDvidFilter::ZDvidFilter() :
  m_minBodySize(0), m_maxBodySize(0), m_hasUpperBodySize(true)
{
}

void ZDvidFilter::exclude(uint64_t bodyId)
{
  m_excludedBodySet.insert(bodyId);
}

void ZDvidFilter::exclude(const std::vector<uint64_t> &bodyArray)
{
  for (auto iter = bodyArray.begin(); iter != bodyArray.end(); ++iter) {
    exclude(*iter);
  }
}

bool ZDvidFilter::isExcluded(uint64_t bodyId) const
{
#ifdef _DEBUG_2
  if (bodyId == 16493) {
    std::cout << "16493: " << m_excludedBodySet.count(bodyId) << std::endl;
  }
#endif
  return m_excludedBodySet.count(bodyId) > 0;
}

bool ZDvidFilter::hasExclusion() const
{
  return !m_excludedBodySet.empty();
}

bool ZDvidFilter::namedBodyOnly() const
{
  return m_namedBodyOnly;
}

bool ZDvidFilter::tracedOnly() const
{
  return m_tracedOnly;
}

std::set<uint64_t> ZDvidFilter::loadBodySet() const
{
//  ZString

  std::set<uint64_t> bodySet;


  if (!(ZFileType::FileType(m_bodyListFile) == ZFileType::EFileType::JSON)) {
    FILE *fp = fopen(m_bodyListFile.c_str(), "r");
    if (fp != NULL) {
      ZString str;
      while (str.readLine(fp)) {
        std::vector<uint64_t> bodyArray = str.toUint64Array();
        bodySet.insert(bodyArray.begin(), bodyArray.end());
      }
      fclose(fp);
    }
  }

  return bodySet;
}
