#ifndef ZGLOBAL_H
#define ZGLOBAL_H

class ZIntPoint;
class ZPoint;
class ZGlobalData;

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

private:
  ZGlobalData *m_data;
};

#endif // ZGLOBAL_H
