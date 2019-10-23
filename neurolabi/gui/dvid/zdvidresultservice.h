#ifndef ZDVIDRESULTSERVICE_H
#define ZDVIDRESULTSERVICE_H

#include <cstdint>

#include <QMutex>
#include <QList>

class QByteArray;
class QString;
class ZObject3dScan;
class ZJsonObject;
class ZDvidReader;
class ZDvidTarget;

class ZDvidResultService
{
public:
  ZDvidResultService();

  static QByteArray ReadData(const QString &path);
  static void WriteData(const QString &path, const QByteArray &data);
  static QList<ZObject3dScan*> ReadSplitResult(const QString &path);
  static QList<ZJsonObject> ReadSplitTaskList(const QString &server);

  static QList<ZObject3dScan*> ReadSplitResult(
      const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId);
  static void RemoveSplitTask(
      const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId);
  static void RemoveSplitResult(
      const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId);

  static bool HasSplitResult(const QString server, const QString taskKey);
  static bool HasSplitResult(const ZDvidReader &reader, const QString taskKey);
  static bool HasSplitResult(const ZDvidReader *reader, const QString taskKey);

//  static QString GetSplitTaskKey(const QString &url);
//  static void WriteTask(const QByteArray &data);

//  static QString GetEndPointKey(const QString &url);
private:
  static QList<ZObject3dScan*> ReadSplitResult(
      const QString &server, const QString &key);
  static ZObject3dScan* ReadSplitObject(
      const ZJsonObject &objJson, ZDvidReader *reader);
  static QString FindSplitResultKey(
      const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId);
  static ZJsonObject ReadHeadObject(
      const ZDvidReader &reader, const QString &dataName, const QString &key);

};

#endif // ZDVIDRESULTSERVICE_H
