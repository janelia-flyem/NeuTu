#include "zdvidwriter.h"
#include <QProcess>
#include <QDebug>
#include "neutubeconfig.h"
#include "flyem/zflyemneuron.h"
#include "zclosedcurve.h"

ZDvidWriter::ZDvidWriter(QObject *parent) :
  QObject(parent)
{
  m_eventLoop = new QEventLoop(this);
  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);
}

bool ZDvidWriter::open(
    const QString &serverAddress, const QString &uuid, int port)
{
  m_dvidClient->reset();

  if (serverAddress.isEmpty()) {
    return false;
  }

  if (uuid.isEmpty()) {
    return false;
  }

  m_dvidClient->setServer(serverAddress, port);
  m_dvidClient->setUuid(uuid);

  return true;
}

bool ZDvidWriter::open(const ZDvidTarget &target)
{
  if (!target.isValid()) {
    return false;
  }

  return open(("http://" + target.getAddress()).c_str(),
              target.getUuid().c_str(), target.getPort());
}

bool ZDvidWriter::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.set(sourceString.toStdString());
  return open(target);
}

void ZDvidWriter::writeSwc(int bodyId, ZSwcTree *tree)
{
  if (tree != NULL && bodyId > 0) {
    QString tmpPath = QString("%1/%2.swc").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(bodyId);
    tree->save(tmpPath.toStdString());
    QString command = QString(
          "curl -X POST %1/api/node/%2/skeletons/%3.swc"
          " --data-binary @%4").arg(m_dvidClient->getServer()).
        arg(m_dvidClient->getUuid()).
        arg(bodyId).arg(tmpPath);

    qDebug() << command;

    QProcess::execute(command);
  }
}

void ZDvidWriter::writeThumbnail(int bodyId, ZStack *stack)
{
  if (stack != NULL && bodyId > 0) {
    QString tmpPath = QString("%1/%2.mraw").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(bodyId);
    stack->save(tmpPath.toStdString());
    QString command = QString(
          "curl -X POST %1/api/node/%2/thumbnails/%3.mraw"
          " --data-binary @%4").arg(m_dvidClient->getServer()).
        arg(m_dvidClient->getUuid()).
        arg(bodyId).arg(tmpPath);

    qDebug() << command;

    QProcess::execute(command);
  }
}

void ZDvidWriter::writeThumbnail(int bodyId, Stack *stack)
{
  if (stack != NULL && bodyId > 0) {
    Mc_Stack mstack;
    C_Stack::view(stack, &mstack);
    ZStack stackObj(&mstack, NULL);

    writeThumbnail(bodyId, &stackObj);
  }
}

void ZDvidWriter::writeAnnotation(int bodyId, const ZJsonObject &obj)
{
  if (bodyId > 0 && !obj.isEmpty()) {
    ZString annotationString = obj.dumpString(0);
    annotationString.replace(" ", "");
    annotationString.replace("\"", "\"\"\"");

    QString command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2/api/node/%3/annotations/%4").arg(annotationString.c_str()).
        arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
        arg(bodyId);

    qDebug() << command;

    QProcess::execute(command);
  }
}

void ZDvidWriter::writeAnnotation(const ZFlyEmNeuron &neuron)
{
  writeAnnotation(neuron.getId(), neuron.getAnnotationJson());
}

void ZDvidWriter::writeRoiCurve(
    const ZClosedCurve &curve, const std::string &key)
{
  if (!key.empty()) {
    ZJsonObject obj = curve.toJsonObject();
    ZString annotationString = obj.dumpString(0);
    annotationString.replace(" ", "");
    annotationString.replace("\"", "\"\"\"");

    QString command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2/api/node/%3/roi_curve/%4").arg(annotationString.c_str()).
        arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
        arg(key.c_str());

    qDebug() << command;

    QProcess::execute(command);
  }
}

void ZDvidWriter::writeJsonString(
    const std::string &dataName, const std::string &key,
    const std::string jsonString)
{
  ZString annotationString = jsonString;
  annotationString.replace(" ", "");
  annotationString.replace("\"", "\"\"\"");

  QString command = QString(
        "curl -g -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2/api/node/%3/%4/%5").arg(annotationString.c_str()).
      arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
      arg(dataName.c_str()).arg(key.c_str());

  qDebug() << command;

  QProcess::execute(command);
}

void ZDvidWriter::writeBoundBox(const ZIntCuboid &cuboid, int z)
{
  ZJsonArray obj;
  obj.append(cuboid.getFirstCorner().getX());
  obj.append(cuboid.getFirstCorner().getY());
  obj.append(cuboid.getFirstCorner().getZ());
  obj.append(cuboid.getLastCorner().getX());
  obj.append(cuboid.getLastCorner().getY());
  obj.append(cuboid.getLastCorner().getZ());

  ZString annotationString = obj.dumpString(0);
  annotationString.replace(" ", "");
  annotationString.replace("\"", "\"\"\"");

  QString command = QString(
        "curl -g -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2/api/node/%3/bound_box/%4").arg(annotationString.c_str()).
      arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
      arg(z);

  qDebug() << command;

  QProcess::execute(command);
}
