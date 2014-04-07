#include "ztextlinecompositer.h"

#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include "zerror.h"

ZTextLineCompositer::ZTextLineCompositer() : m_level(0)
{
}

void ZTextLineCompositer::print(int indent, int level) const
{
  std::cout << toString(indent, level);
}

std::string ZTextLineCompositer::toString(int indent, int level) const
{
  std::ostringstream stream;

  for (std::vector<std::string>::const_iterator iter = m_lineArray.begin();
      iter != m_lineArray.end(); ++iter) {
    stream << std::setfill(' ') << std::setw(indent * (m_level + level)) << "";
    stream << *iter << std::endl;
  }

  for (std::vector<ZTextLineCompositer>::const_iterator
       iter = m_compositerArray.begin(); iter != m_compositerArray.end();
       ++iter) {
    const ZTextLineCompositer& compositer = *iter;
    stream << compositer.toString(indent, level + m_level);
  }

  return stream.str();
}

void ZTextLineCompositer::appendLine(const std::string &line, int level)
{
  PROCESS_WARNING(level < 0, "Negative level", return);

  if (level == 0 && m_compositerArray.empty()) {
    m_lineArray.push_back(line);
  } else {
    ZTextLineCompositer compositer;
    compositer.appendLine(line);
    appendLine(compositer, level);
  }
}

void ZTextLineCompositer::appendLine(const ZTextLineCompositer &compositer,
                                     int level)
{
  PROCESS_WARNING(level < 0, "Negative level", return);
  m_compositerArray.push_back(compositer);
  m_compositerArray.back().setLevel(level);
}

