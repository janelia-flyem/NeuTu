#include "zdvidfilter.h"
#include <iostream>
#include "zstring.h"
#include "zfiletype.h"

ZDvidFilter::ZDvidFilter() :
  m_minBodySize(0), m_maxBodySize(0), m_hasUpperBodySize(true)
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

std::set<uint64_t> ZDvidFilter::loadBodySet() const
{
//  ZString

  std::set<uint64_t> bodySet;


  if (!(ZFileType::fileType(m_bodyListFile) == ZFileType::JSON_FILE)) {
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
