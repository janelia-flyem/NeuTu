//#define _NEUTU_USE_REF_KEY_
#include "zserviceconsumer.h"

#include <QByteArray>
#include <QString>
#include <QUrl>

#include "zjsondef.h"
#include "dvid/zdvidurl.h"
#include "zglobal.h"
#include "zdvidutil.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"

//const char* ZServiceConsumer::REF_KEY = "->";

ZServiceConsumer::ZServiceConsumer()
{

}

QByteArray ZServiceConsumer::ReadData(const QString &path)
{
  QByteArray data;

  QUrl url(path);
  if (url.scheme() == "http") {
    ZDvidTarget target = ZDvid::MakeTargetFromUrl(path.toStdString());
    if (target.isValid()) {
      ZDvidReader *reader =
          ZGlobal::GetInstance().getDvidReader(target.getSourceString(true));
      data = reader->readDataFromEndpoint(
            ZDvidUrl::GetPath(url.path().toStdString()));
    }
  }

  return data;
}

void ZServiceConsumer::WriteData(const QString &path, const QByteArray &data)
{
  QUrl url(path);
  if (url.scheme() == "http") {
    ZDvidTarget target = ZDvid::MakeTargetFromUrl(path.toStdString());
    if (target.isValid()) {
      ZDvidWriter *writer =
          ZGlobal::GetInstance().getDvidWriter(target.getSourceString(true));
      writer->writeData(path.toStdString(), data);
    }
  }
}

ZObject3dScan* ZServiceConsumer::ReadSplitObject(
    const ZJsonObject &objJson, ZDvidReader *reader)
{
  ZObject3dScan *obj = NULL;
  if (objJson.hasKey(neutube::json::REF_KEY) && objJson.hasKey("label")) {
    QByteArray regionData = reader->readDataFromEndpoint(
          ZJsonParser::stringValue(objJson[neutube::json::REF_KEY]));
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

bool ZServiceConsumer::HasSplitResult(
    const ZDvidReader &reader, const QString taskKey)
{
  return reader.hasKey(
        ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
        ZDvidUrl::GetResultKeyFromTaskKey(taskKey.toStdString()).c_str());
}

bool ZServiceConsumer::HasSplitResult(
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

bool ZServiceConsumer::HasSplitTask(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(server.toStdString());

  bool hasTask = false;
  if (reader != NULL) {
    hasTask = reader->hasKey(
          ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
          ZDvidUrl(bodySource).getSplitTaskKey(bodyId).c_str());
  }

  return hasTask;
}

bool ZServiceConsumer::HasSplitResult(
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

bool ZServiceConsumer::HasNonemptySplitResult(
    const ZDvidReader *reader, const QString taskKey)
{
  bool hasResult = false;
  if (reader != NULL) {
    QByteArray data = reader->readKeyValue(
          ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY),
          ZDvidUrl::GetResultKeyFromTaskKey(taskKey.toStdString()).c_str());
    if (!data.isEmpty()) {
      ZJsonObject jsonObj;
      jsonObj.decodeString(data.constData());
      if (jsonObj.hasKey("committed")) {
        ZJsonArray commitArray(jsonObj.value("committed"));
        if (commitArray.size() > 0) {
          hasResult = true;
        }
      }
    }
  }
  return hasResult;
}

QList<ZStackObject*> ZServiceConsumer::ReadSplitTaskSeed(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(server.toStdString());

  QList<ZStackObject*> seedList;
  seedList = reader->readSeedFromSplitTask(bodySource, bodyId);
  return seedList;
}

QList<ZObject3dScan*> ZServiceConsumer::ReadSplitResult(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidUrl dvidUrl(bodySource);
  std::string taskKey = dvidUrl.getSplitTaskKey(bodyId);
  std::string resultKey = ZDvidUrl::GetResultKeyFromTaskKey(taskKey);
//  QString resultKey = FindSplitResultKey(server, bodySource, bodyId);

  QList<ZObject3dScan*> objList = ReadSplitResult(server, resultKey.c_str());

  return objList;
}

void ZServiceConsumer::RemoveSplitResult(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidUrl dvidUrl(bodySource);
  QString resultKey(dvidUrl.getSplitResultKey(bodyId).c_str());
//  QString resultKey = FindSplitResultKey(server, bodySource, bodyId);
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(server.toStdString());
  writer->deleteKey(
        ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_RESULT_KEY), resultKey);
}

void ZServiceConsumer::RemoveSplitTask(
    const QString &server, const ZDvidTarget &bodySource, uint64_t bodyId)
{
  ZDvidUrl url(bodySource);
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriterFromUrl(server.toStdString());
  writer->deleteKey(ZDvidData::GetName(ZDvidData::ERole::SPLIT_TASK_KEY),
                    url.getSplitTaskKey(bodyId));
}

ZJsonObject ZServiceConsumer::ReadHeadObject(
      const ZDvidReader &reader, const QString &dataName, const QString &key)
{
  ZJsonObject obj = reader.readJsonObjectFromKey(dataName, key);
  if (obj.hasKey(neutube::json::REF_KEY)) {
    obj = reader.readJsonObject(
          ZJsonParser::stringValue(obj[neutube::json::REF_KEY]));
  }

  return obj;
}
#if 0
QString ZServiceConsumer::FindSplitResultKey(
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
#endif

QList<ZObject3dScan*> ZServiceConsumer::ReadSplitResult(
    const QString &server, const QString &key)
{
  ZDvidTarget target;
  target.setFromUrl(server.toStdString());
  ZDvidUrl dvidUrl(target);
  QString path(dvidUrl.getKeyUrl("result_split", key.toStdString()).c_str());

  return ZServiceConsumer::ReadSplitResult(path);
}


QList<ZObject3dScan*> ZServiceConsumer::ReadSplitResult(const QString &path)
{
  QList<ZObject3dScan*> objList;
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(path.toStdString());
  if (reader != NULL) {
    QByteArray data = reader->readBuffer(path.toStdString());
    ZJsonObject obj;
    if (data[0] == '{') {
      obj.decodeString(QString::fromLocal8Bit(data.data(), data.length()).toLocal8Bit());
    }

    if (!obj.isEmpty()) {
      ZJsonObject headJson = obj;
      if (obj.hasKey(neutube::json::REF_KEY)) {
        std::string refPath = ZJsonParser::stringValue(obj[neutube::json::REF_KEY]);
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

QList<ZJsonObject> ZServiceConsumer::ReadSplitTaskList(const QString &server)
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
