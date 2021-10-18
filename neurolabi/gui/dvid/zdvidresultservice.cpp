#include "zdvidresultservice.h"

#include <QByteArray>
#include <QString>
#include <QUrl>

#include "dvid/zdvidurl.h"
#include "zglobal.h"
#include "zdvidutil.h"
#include "zdvidreader.h"
#include "zdvidwriter.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"
#include "zjsonparser.h"

ZDvidResultService::ZDvidResultService()
{

}

QByteArray ZDvidResultService::ReadData(const QString &path)
{
  QByteArray data;

  QUrl url(path);
  if (url.scheme() == "http") {
    ZDvidTarget target = dvid::MakeTargetFromUrl_deprecated(path.toStdString());
    if (target.isValid()) {
      ZDvidReader *reader =
          ZGlobal::GetInstance().getDvidReader(target.getSourceString(true));
      data = reader->readDataFromEndpoint(
            ZDvidUrl::GetPath(url.path().toStdString()));
    }
  }

  return data;
}

void ZDvidResultService::WriteData(const QString &path, const QByteArray &data)
{
  QUrl url(path);
  if (url.scheme() == "http") {
    ZDvidTarget target = dvid::MakeTargetFromUrl_deprecated(path.toStdString());
    if (target.isValid()) {
      ZDvidWriter *writer =
          ZGlobal::GetInstance().getDvidWriter(target.getSourceString(true));
      writer->writeData(path.toStdString(), data);
    }
  }
}

ZObject3dScan* ZDvidResultService::ReadSplitObject(
    const ZJsonObject &objJson, ZDvidReader *reader)
{
  ZObject3dScan *obj = NULL;
  if (objJson.hasKey("ref") && objJson.hasKey("label")) {
    QByteArray regionData = reader->readDataFromEndpoint(
          ZJsonParser::stringValue(objJson["ref"]));
    if (!regionData.isEmpty()) {
      obj = new ZObject3dScan;
      obj->importDvidObjectBuffer(
            regionData.data(), regionData.length());
      if (!obj->isEmpty()) {
        obj->setLabel(ZJsonParser::integerValue(objJson["label"]));
      } else {
        delete obj;
        obj = NULL;
      }
    }
  }

  return obj;
}

bool ZDvidResultService::HasSplitResult(
    const ZDvidReader &reader, const QString taskKey)
{
  return reader.hasKey(
        ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
        ZDvidUrl::GetResultKeyFromTaskKey(taskKey.toStdString()).c_str());
}

bool ZDvidResultService::HasSplitResult(
    const ZDvidReader *reader, const QString taskKey)
{
  bool hasResult = false;
  if (reader != NULL) {
    hasResult = reader->hasKey(
        ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
        ZDvidUrl::GetResultKeyFromTaskKey(taskKey.toStdString()).c_str());
  }
  return hasResult;
}

bool ZDvidResultService::HasSplitResult(
    const QString server, const QString taskKey)
{
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(server.toStdString());

  bool hasResult = false;
  if (reader != NULL) {
    hasResult = reader->hasKey(
          ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
          ZDvidUrl::GetResultKeyFromTaskKey(taskKey.toStdString()).c_str());
  }

  return hasResult;
}

QList<ZObject3dScan*> ZDvidResultService::ReadSplitResult(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  QString resultKey = FindSplitResultKey(server, bodySource, bodyId);

  QList<ZObject3dScan*> objList = ReadSplitResult(server, resultKey);

  return objList;
}

void ZDvidResultService::RemoveSplitResult(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  QString resultKey = FindSplitResultKey(server, bodySource, bodyId);
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(server.toStdString());
  writer->deleteKey(
        ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY), resultKey);
}

void ZDvidResultService::RemoveSplitTask(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidUrl url(bodySource);
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(server.toStdString());
  writer->deleteKey(ZDvidData::GetName(ZDvidData::ERole::SPLIT_TASK_KEY),
                    url.getSplitTaskKey(bodyId));
}

ZJsonObject ZDvidResultService::ReadHeadObject(
      const ZDvidReader &reader, const QString &dataName, const QString &key)
{
  ZJsonObject obj = reader.readJsonObjectFromKey(dataName, key);
  if (obj.hasKey("ref")) {
    obj = reader.readJsonObject(ZJsonParser::stringValue(obj["ref"]));
  }

  return obj;
}

QString ZDvidResultService::FindSplitResultKey(
      const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  QString resultKey;

  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(server.toStdString());
  if (reader != NULL) {
    std::map<std::string, ZJsonObject> taskMap = reader->readSplitTaskMap();
    for (std::map<std::string, ZJsonObject>::const_iterator iter = taskMap.begin();
         iter != taskMap.end(); ++iter) {
      const std::string key = iter->first;
      const ZJsonObject &taskJson = iter->second;
      if (taskJson.hasKey("signal")) {
        std::string signalPath =
            ZJsonParser::stringValue(taskJson["signal"]);

#ifdef _DEBUG_
        std::cout << signalPath << std::endl;
#endif

        ZDvidUrl dvidUrl(bodySource);
        if (QUrl(dvidUrl.getSparsevolUrl(bodyId).c_str()) ==
            QUrl(signalPath.c_str())) {
          resultKey = ZDvidUrl::GetResultKeyFromTaskKey(key).c_str();
          break;
        }
      }
    }
  }

  return resultKey;
}

QList<ZObject3dScan*> ZDvidResultService::ReadSplitResult(
    const QString &server, const QString &key)
{
  ZDvidTarget target;
  target.setFromUrl_deprecated(server.toStdString());
  ZDvidUrl dvidUrl(target);
  QString path(dvidUrl.getKeyUrl("result_split", key.toStdString()).c_str());

  return ZDvidResultService::ReadSplitResult(path);
}

QList<ZObject3dScan*> ZDvidResultService::ReadSplitResult(const QString &path)
{
  QList<ZObject3dScan*> objList;
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(path.toStdString());
  if (reader != NULL) {
    QByteArray data = reader->readBuffer(path.toStdString());
    ZJsonObject obj;
    if (data[0] == '{') {
      obj.decodeString(
            QString::fromLocal8Bit(data.data(), data.length()).toLocal8Bit());
    }

    if (!obj.isEmpty()) {
      ZJsonObject headJson = obj;
      if (obj.hasKey("ref")) {
        std::string refPath = ZJsonParser::stringValue(obj["ref"]);
        data = reader->readDataFromEndpoint(refPath);
        if (data[0] == '{') {
          headJson.decodeString(
                QString::fromLocal8Bit(data.data(), data.length()).toLocal8Bit());
        }
      }

      if (std::string(ZJsonParser::stringValue(headJson["type"])) == "split") {
        if (headJson.hasKey("result")) {
          ZJsonArray resultJson(headJson.value("result"));
          for (size_t i = 0; i < resultJson.size(); ++i) {
            ZJsonObject objJson(resultJson.value(i));
            ZObject3dScan *obj = ReadSplitObject(objJson, reader);
            if (obj != NULL) {
              objList.append(obj);
            }
          }
        }
      }
    }
  }

  return objList;
}

QList<ZJsonObject> ZDvidResultService::ReadSplitTaskList(const QString &server)
{
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(server.toStdString());
  QStringList keyList = reader->readKeys("task_split", "task__0", "task__z");
  QList<ZJsonObject> taskList;
  foreach (const QString &key, keyList) {
    ZJsonObject obj = ReadHeadObject(
          *reader, ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_TASK_KEY), key);
    /*
    QByteArray data = reader->readKeyValue("task_split", key);
    ZJsonObject obj;
    obj.decodeString(data.constData());
    */
    if (!obj.isEmpty()) {
      taskList.append(obj);
    }
  }

  return taskList;
}

/*
QString ZDvidResultService::GetSplitTaskKey(const QString &url)
{
  QString w = "task_split/key/";
  int index = url.lastIndexOf(w);

  return QStringRef(url, index + w.size(), );
}
QString ZDvidResultService::GetEndPointKey(const QString &url)
{
  QString w = "/key/";
  int index = url.lastIndexOf(w);

  return QStringRef(url, index + w.size());
}
*/
