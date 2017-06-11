#ifndef ZGLOBAL_H
#define ZGLOBAL_H

#include <string>
#include <map>

class ZIntPoint;
class ZPoint;
class ZGlobalData;
class ZDvidReader;
class ZDvidWriter;
class ZDvidSparseStack;
class ZDvidTarget;

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
  ZDvidReader* getDvidReader(const ZDvidTarget &target) const;
  ZDvidWriter* getDvidWriter(const ZDvidTarget &target) const;
  ZDvidReader* getDvidReaderFromUrl(const std::string &url) const;
  ZDvidWriter* getDvidWriterFromUrl(const std::string &url) const;

public:
  ZDvidSparseStack* readDvidSparseStack(const std::string &url) const;

private:
  template<typename T>
  T* getIODevice(
      const std::string &name, std::map<std::string, T*> &ioMap) const;

  template<typename T>
  T* getIODevice(
      const ZDvidTarget &name, std::map<std::string, T*> &ioMap) const;

  template<typename T>
  T* getIODeviceFromUrl(
      const std::string &path, std::map<std::string, T*> &ioMap) const;

private:
  ZGlobalData *m_data;
};

#endif // ZGLOBAL_H
