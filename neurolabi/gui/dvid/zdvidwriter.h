#ifndef ZDVIDWRITER_H
#define ZDVIDWRITER_H

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
#include "zintcuboid.h"
#include "zsparsestack.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"

class ZFlyEmNeuron;
class ZClosedCurve;
class ZIntCuboid;

class ZDvidWriter : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidWriter(QObject *parent = 0);

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  void writeSwc(int bodyId, ZSwcTree *tree);
  void writeThumbnail(int bodyId, ZStack *stack);
  void writeThumbnail(int bodyId, Stack *stack);
  void writeAnnotation(int bodyId, const ZJsonObject &obj);
  void writeAnnotation(const ZFlyEmNeuron &neuron);
  void writeRoiCurve(const ZClosedCurve &curve, const std::string &key);
  void writeJsonString(const std::string &dataName, const std::string &key,
                       const std::string jsonString);
  void writeBoundBox(const ZIntCuboid &cuboid, int z);

private:
  QEventLoop *m_eventLoop;
  ZDvidClient *m_dvidClient;
  QTimer *m_timer;
};

#endif // ZDVIDWRITER_H
