#ifndef ZDVIDREADER_H
#define ZDVIDREADER_H

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <string>
#include <vector>
#include "zobject3dscan.h"
#include "zswctree.h"
#include "zstack.hxx"
#include "zdvidclient.h"
#include "flyem/zflyem.h"

class ZDvidTarget;

class ZDvidReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidReader(QObject *parent = 0);

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  ZSwcTree *readSwc(int bodyId);
  ZObject3dScan readBody(int bodyId);
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth);
  ZStack* readBodyLabel(
      int x0, int y0, int z0, int width, int height, int depth);
  QString readInfo(const QString &dataType);
  std::set<int> readBodyId(
      int x0, int y0, int z0, int width, int height, int depth);
  std::set<int> readBodyId(const ZIntPoint &firstCorner,
                              const ZIntPoint &lastCorner);
  std::set<int> readBodyId(size_t minSize, size_t maxSize);
  QByteArray readKeyValue(const QString &dataName, const QString &key);

signals:

public slots:
  void slotTest();

private:
  static std::vector<std::pair<int, int> > partitionStack(
      int x0, int y0, int z0, int width, int height, int depth);

private:
  QEventLoop *m_eventLoop;
  ZDvidClient *m_dvidClient;
  QTimer *m_timer;
};

#endif // ZDVIDREADER_H
