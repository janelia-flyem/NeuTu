#include "zdvidwriter.h"
#include <QProcess>
#include <QDebug>
#include "neutubeconfig.h"

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
