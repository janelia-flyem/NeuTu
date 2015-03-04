#ifndef ZTEXTLINECOMPOSITER_H
#define ZTEXTLINECOMPOSITER_H

#include <vector>
#include <string>

class ZTextLineCompositer
{
public:
  ZTextLineCompositer();

  inline void setLevel(int level) { m_level = level; }
  inline int getLevel() const { return m_level; }

  std::string toString(int indent, int level = 0) const;
  void print (int indent, int level = 0) const;

  void appendLine(const std::string &line, int level = 0);
  void appendLine(const ZTextLineCompositer &compositer, int level = 0);

private:
  std::vector<std::string> m_lineArray;
  std::vector<ZTextLineCompositer> m_compositerArray;
  int m_level; //relative level
};

#endif // ZTEXTLINECOMPOSITER_H
