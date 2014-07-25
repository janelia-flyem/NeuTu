#include "zdvidwriter.h"
#include <QProcess>
#include <QDebug>
#include "neutubeconfig.h"
#include "flyem/zflyemneuron.h"

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
