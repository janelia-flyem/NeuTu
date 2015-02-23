#ifndef ZDVIDRESOLUTION_H
#define ZDVIDRESOLUTION_H

class ZDvidResolution
{
public:
  ZDvidResolution();

  int getScale() const;

  inline int getLevel() const { return m_level; }
  inline void setLevel(int level) { m_level = level; }

  void print() const;

private:
  int m_level;
};

#endif // ZDVIDRESOLUTION_H
