#include "zdvidresultservice.h"

#include <QByteArray>
#include <QString>
#include <QUrl>

#include "dvid/zdvidurl.h"
#include "zglobal.h"
#include "zdvidutil.h"
#include "zdvidreader.h"
#include "zdvidwriter.h"

ZDvidResultService::ZDvidResultService()
{

}

QByteArray ZDvidResultService::ReadData(const QString &path)
{
  QByteArray data;

  QUrl url(path);
  if (url.scheme() == "http") {
    ZDvidTarget target = ZDvid::MakeTargetFromUrl(path.toStdString());
    if (target.isValid()) {
      ZDvidReader *reader =
          ZGlobal::GetInstance().getDvidReader(target.getSourceString(true));
      data = reader->readDataFromEndpoint(
            ZDvidUrl::GetEndPoint(url.path().toStdString()));
    }
  }

  return data;
}

void ZDvidResultService::WriteData(const QString &path, const QByteArray &data)
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

QList<ZObject3dScan*> ZDvidResultService::ReadSplitResult(const QString &path)
{
  QList<ZObject3dScan*> objList;
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReaderFromUrl(path.toStdString());
  if (reader != NULL) {
    QByteArray data = reader->readBuffer(path.toStdString());
    ZJsonObject obj;
    if (data[0] == '{') {
      obj.decodeString(QString().fromAscii(data.data(), data.length()).toLocal8Bit());
    }

    if (!obj.isEmpty()) {
      if (std::string(ZJsonParser::stringValue(obj["type"])) == "split") {
        if (obj.hasKey("result")) {
          ZJsonArray resultJson(obj.value("result"));
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
