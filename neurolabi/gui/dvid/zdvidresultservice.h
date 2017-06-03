#ifndef ZDVIDRESULTSERVICE_H
#define ZDVIDRESULTSERVICE_H

#include <QMutex>
#include <QList>

class QByteArray;
class QString;
class ZObject3dScan;
class ZJsonObject;
class ZDvidReader;

class ZDvidResultService
{
public:
  ZDvidResultService();

  static QByteArray ReadData(const QString &path);
  static void WriteData(const QString &path, const QByteArray &data);
  static QList<ZObject3dScan*> ReadSplitResult(const QString &path);

//  static void WriteTask(const QByteArray &data);

private:
  static ZObject3dScan* ReadSplitObject(
      const ZJsonObject &objJson, ZDvidReader *reader);

};

#endif // ZDVIDRESULTSERVICE_H
