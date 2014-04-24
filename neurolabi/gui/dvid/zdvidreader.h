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

class ZDvidReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidReader(QObject *parent = 0);

  void open(const QString &serverAddress, const QString &uuid,
            int port = -1);

  ZSwcTree *readSwc(int bodyId);
  ZObject3dScan readBody(int bodyId);
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth);
  ZStack* readLabelMap(
      int x0, int y0, int z0, int width, int height, int depth);
  QString readInfo(const QString &dataType);
  std::vector<int> readBodyId(
      int x0, int y0, int z0, int width, int height, int depth);

signals:

public slots:
  void slotTest();

private:
  QEventLoop *m_eventLoop;
  ZDvidClient *m_dvidClient;
  QTimer *m_timer;
};

#endif // ZDVIDREADER_H
