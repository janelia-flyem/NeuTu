#ifndef ZGLOBAL_H
#define ZGLOBAL_H

#include <string>
#include <map>

class ZIntPoint;
class ZPoint;
class ZGlobalData;
class ZDvidReader;
class ZDvidWriter;

class ZGlobal
{
public:
  ZGlobal();
  ~ZGlobal();

  static ZGlobal& GetInstance() {
    static ZGlobal g;

    return g;
  }

  void setStackPosition(int x, int y, int z);
  void setStackPosition(const ZIntPoint &pt);
  void setStackPosition(const ZPoint &pt);
  void clearStackPosition();
  ZIntPoint getStackPosition() const;
  ZDvidReader* getDvidReader(const std::string &name) const;
  ZDvidWriter* getDvidWriter(const std::string &name) const;

private:
  template<typename T>
  T* getIODevice(
      const std::string &name, std::map<std::string, T*> &ioMap) const;

private:
  ZGlobalData *m_data;
};

#endif // ZGLOBAL_H
