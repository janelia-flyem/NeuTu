#include "zskeletonizeservice.h"
#include <QProcess>
#include <QDebug>
#include "dvid/zdvidtarget.h"

ZSkeletonizeService::ZSkeletonizeService()
{
  m_server = "emrecon100.janelia.priv:9082";
}

void ZSkeletonizeService::callService(const ZDvidTarget &target, int bodyId)
{
  QProcess process;
  if (target.isValid()) {
    QString command = "curl";
    QString data = QString(
          "{\"dvid-server\": \"%1\", \"uuid\": \"%2\", \"bodies\": [%3]}").
        arg(target.getAddressWithPort().c_str()).
        arg(target.getUuid().c_str()).arg(bodyId);
    QStringList args;
    args << "-X" << "POST" << "-H" << "Content-Type: application/json"
         << "-d" << data
         << QString("http://%4/skeletonize").arg(m_server);

    qDebug() << command;
    qDebug() << args;
    process.start(command, args);
    process.waitForFinished(300000);
  }
}
