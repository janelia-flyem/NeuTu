#include "zdvidwriter.h"
#include <iostream>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "neutubeconfig.h"
#include "flyem/zflyemneuron.h"
#include "zclosedcurve.h"
#include "zswctree.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdviddata.h"
#include "dvid/zdvidreader.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "zerror.h"
#include "zjsonfactory.h"

#if _ENABLE_LIBDVID_
#include "DVIDNode.h"
#endif

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
  m_dvidTarget.set(serverAddress.toStdString(), uuid.toStdString(), port);

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

  m_dvidTarget = target;
  m_dvidClient->reset();
  m_dvidClient->setDvidTarget(target);

  return true;
}

bool ZDvidWriter::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString.toStdString());
  return open(target);
}

void ZDvidWriter::writeSwc(int bodyId, ZSwcTree *tree)
{
  if (tree != NULL && bodyId > 0) {
    QString tmpPath = QString("%1/%2.swc").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(bodyId);
    tree->save(tmpPath.toStdString());

    ZDvidUrl dvidUrl(m_dvidTarget);
    QString command = QString("curl -X POST %1 --data-binary @%2").
        arg(dvidUrl.getSkeletonUrl(
              bodyId, m_dvidTarget.getBodyLabelName()).c_str()).arg(tmpPath);
    /*
    QString command = QString(
          "curl -X POST %1/api/node/%2/skeletons/%3.swc"
          " --data-binary @%4").arg(m_dvidClient->getServer()).
        arg(m_dvidClient->getUuid()).
        arg(bodyId).arg(tmpPath);
        */

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
    ZDvidUrl dvidUrl(m_dvidTarget);

    QString command = QString("curl -X POST %1 --data-binary @%2").
        arg(dvidUrl.getThumbnailUrl(
              bodyId, m_dvidTarget.getBodyLabelName()).c_str()).
        arg(tmpPath);

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
    //ZString annotationString = obj.dumpString(0);
    //annotationString.replace(" ", "");
    //annotationString.replace("\"", "\"\"\"");

    /*
    QString command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2/api/node/%3/annotations/%4").arg(annotationString.c_str()).
        arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
        arg(bodyId);
        */

    QString command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2").arg(getJsonStringForCurl(obj).c_str()).
        arg(ZDvidUrl(m_dvidTarget).getAnnotationUrl(
              bodyId, m_dvidTarget.getBodyLabelName()).c_str());

    qDebug() << command;

    QProcess::execute(command);
  }
}

void ZDvidWriter::writeAnnotation(const ZFlyEmNeuron &neuron)
{
  writeAnnotation(neuron.getId(), neuron.getAnnotationJson());
}

void ZDvidWriter::writeBodyInfo(int bodyId, const ZJsonObject &obj)
{
  if (bodyId > 0 && !obj.isEmpty()) {
    writeJsonString(ZDvidData::GetName(ZDvidData::ROLE_BODY_INFO,
                                       ZDvidData::ROLE_BODY_LABEL,
                                       m_dvidTarget.getBodyLabelName()),
                    ZString::num2str(bodyId).c_str(),
                    obj.dumpString(0).c_str());
  }
}

void ZDvidWriter::writeRoiCurve(
    const ZClosedCurve &curve, const std::string &key)
{
  if (!key.empty()) {
    ZJsonObject obj = curve.toJsonObject();
    /*
    ZString annotationString = obj.dumpString(0);
    annotationString.replace(" ", "");
    annotationString.replace("\"", "\"\"\"");

    QString command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2/api/node/%3/roi_curve/%4").arg(annotationString.c_str()).
        arg(m_dvidClient->getServer()).arg(m_dvidClient->getUuid()).
        arg(key.c_str());
        */

    writeJson(ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE), key, obj);

    /*
    QString command =
        QString("curl -g -X POST -H \"Content-Type: application/json\" "
                "-d \"%1\" %2").arg(getJsonStringForCurl(obj).c_str()).
        arg(ZDvidUrl(m_dvidTarget).getKeyUrl(
              ZDvidData::getName(ZDvidData::ROLE_ROI_CURVE), key).c_str());

    qDebug() << command;

    QProcess::execute(command);
    */
  }
}

void ZDvidWriter::deleteRoiCurve(const std::string &key)
{
  if (!key.empty()) {
    deleteKey(ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE), key);
  }
}

void ZDvidWriter::writeJsonString(
    const std::string &dataName, const std::string &key,
    const std::string jsonString)
{
  writeJsonString(ZDvidUrl(m_dvidTarget).getKeyUrl(dataName, key), jsonString);
}

void ZDvidWriter::writeJson(
    const std::string &dataName, const std::string &key, const ZJsonValue &obj)
{
  writeJsonString(dataName, key, obj.dumpString(0));
}

void ZDvidWriter::writeJsonString(
    const std::string url, const std::string jsonString)
{
  QString annotationString = jsonString.c_str();

  QString command;
  if (annotationString.size() < 5000) {
    annotationString.replace(" ", "");
    annotationString.replace("\"", "\"\"\"");

    command = QString(
          "curl -g -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2").arg(annotationString).
        arg(url.c_str());
  } else {
    QString urlStr(url.c_str());
    urlStr.replace("/", "_");
    urlStr.replace(":", "_");
    QString tmpPath = QString("%1/%2.json").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(urlStr);
    QFile file(tmpPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << annotationString;
    file.close();

    command = QString("curl -g -X POST -H \"Content-Type: application/json\" "
                      "-d \"@%1\" %2").arg(tmpPath).arg(url.c_str());
  }

  qDebug() << command;

  QProcess::execute(command);
}


void ZDvidWriter::writeJson(const std::string url, const ZJsonValue &value)
{
  writeJsonString(url, value.dumpString(0));
}

void ZDvidWriter::mergeBody(const std::string &dataName,
                            int targetId, const std::vector<int> &bodyId)
{
  ZJsonArray jsonArray(json_array(), ZJsonValue::SET_AS_IT_IS);
  jsonArray.append(targetId);
  for (std::vector<int>::const_iterator iter = bodyId.begin();
       iter != bodyId.end(); ++iter) {
    jsonArray.append(*iter);
  }

  /*
  ZJsonArray mergeArray(json_array(), ZJsonValue::SET_AS_IT_IS);
  mergeArray.append(jsonArray);
*/
  ZDvidUrl dvidUrl(m_dvidTarget);
  writeJson(dvidUrl.getMergeUrl(dataName), jsonArray);
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

  //ZString annotationString = obj.dumpString(0);
  //annotationString.replace(" ", "");
  //annotationString.replace("\"", "\"\"\"");

  QString command = QString(
        "curl -g -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2").arg(getJsonStringForCurl(obj).c_str()).
      arg(ZDvidUrl(m_dvidTarget).getBoundBoxUrl(z).c_str());

  qDebug() << command;

  QProcess::execute(command);
}
/*
void ZDvidWriter::writeSplitLabel(const ZObject3dScan &obj, int label)
{

}
*/
void ZDvidWriter::createKeyvalue(const std::string &name)
{
  createData("keyvalue", name);
}

std::string ZDvidWriter::getJsonStringForCurl(const ZJsonValue &obj) const
{
  ZString jsonString = obj.dumpString(0);
  jsonString.replace(" ", "");
  jsonString.replace("\"", "\"\"\"");

  return jsonString;
}

void ZDvidWriter::createData(const std::string &type, const std::string &name)
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject obj;
  obj.setEntry("typename", type);
  obj.setEntry("dataname", name);

  QString command = QString(
        "curl -X POST -H \"Content-Type: application/json\" -d \"%1\" %2").
      arg(getJsonStringForCurl(obj).c_str()).
      arg(dvidUrl.getInstanceUrl().c_str());

  qDebug() << command;

  QProcess::execute(command);
}

void ZDvidWriter::deleteKey(const std::string &dataName, const std::string &key)
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  std::string url = dvidUrl.getKeyUrl(dataName, key);
  QString command = QString("curl -X DELETE %1").arg(url.c_str());
  qDebug() << command;

  QProcess::execute(command);
}

void ZDvidWriter::deleteKey(const QString &dataName, const QString &key)
{
  deleteKey(dataName.toStdString(), key.toStdString());
}

void ZDvidWriter::deleteKey(const std::string &dataName,
               const std::string &minKey, const std::string &maxKey)
{
  deleteKey(QString(dataName.c_str()), minKey.c_str(), maxKey.c_str());
}

void ZDvidWriter::deleteKey(const QString &dataName, const QString &minKey,
    const QString &maxKey)
{
  ZDvidReader reader;
  if (reader.open(m_dvidTarget)) {
    QStringList keyList = reader.readKeys(dataName, minKey, maxKey);
    foreach (const QString& key, keyList) {
      deleteKey(dataName, key);
    }
  }
}

void ZDvidWriter::writeBodyInfo(int bodyId)
{
  ZDvidReader reader;
  if (reader.open(m_dvidTarget)) {
    ZObject3dScan obj;
    reader.readBody(bodyId, &obj);
    if (!obj.isEmpty()) {
      ZFlyEmNeuronBodyInfo bodyInfo;
      bodyInfo.setBodySize(obj.getVoxelNumber());
      bodyInfo.setBoundBox(obj.getBoundBox());
      ZJsonObject obj = bodyInfo.toJsonObject();
      ZDvidUrl dvidUrl(m_dvidTarget);
      writeJson(dvidUrl.getBodyInfoUrl(
                  bodyId, m_dvidTarget.getBodyLabelName()), obj);
    }
  }
}

#if 0
void ZDvidWriter::writeMaxBodyId(int bodyId)
{
  ZJsonObject idJson;
  idJson.setEntry("max_body_id", bodyId);
  ZDvidUrl dvidUrl(m_dvidTarget);
  writeJson(dvidUrl.getMaxBodyIdUrl(), idJson);
}
#endif

bool ZDvidWriter::lockNode(const std::string &message)
{
  ZJsonObject messageJson;
  ZJsonArray messageArrayJson;
  messageArrayJson.append(message);
  messageJson.setEntry("log", messageArrayJson);

  QString command = QString(
        "curl -g -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2").arg(getJsonStringForCurl(messageJson).c_str()).
      arg(ZDvidUrl(m_dvidTarget).getLockUrl().c_str());

  qDebug() << command;

  QProcess::execute(command);

  return true;
}

std::string ZDvidWriter::createBranch()
{
  std::string uuid;

  QProcess process;

  QString command = QString("curl -X POST %21").
      arg(ZDvidUrl(m_dvidTarget).getBranchUrl().c_str());
  process.start(command);
  if (!process.waitForFinished(-1)) {
    RECORD_ERROR_UNCOND(process.errorString().toStdString());
  } else {
    QByteArray buffer = process.readAllStandardOutput();

#ifdef _DEBUG_
    std::cout << QString(buffer.data()).toStdString() << std::endl;
#endif

    ZJsonObject obj;
    obj.decodeString(buffer.data());

    if (obj.hasKey("child")) {
      uuid = ZJsonParser::stringValue(obj["child"]);
    }
  }

  return uuid;
}

void ZDvidWriter::writeSplit(
    const std::string &dataName, const ZObject3dScan &obj,
    uint64_t oldLabel, uint64_t label)
{
  //POST <api URL>/node/<UUID>/<data name>/split/<label>
  QString tmpPath = QString("%1/%2_%3.dvid").
      //arg((GET_TEST_DATA_DIR + "/backup").c_str()).
      arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
      arg(oldLabel).arg(label);

  obj.exportDvidObject(tmpPath.toStdString());

#ifdef _DEBUG_
  std::cout << tmpPath.toStdString() + " saved" << std::endl;
#endif

  QString command = QString(
        "curl -X POST %1 --data-binary \"@%2\"").
      arg(ZDvidUrl(m_dvidTarget).getSplitUrl(dataName, oldLabel).c_str()).
      arg(tmpPath);

  qDebug() << command;

  QProcess::execute(command);

}

void ZDvidWriter::writeMergeOperation(const QMap<uint64_t, uint64_t> &bodyMap)
{
  std::string url = ZDvidUrl(m_dvidTarget).getMergeOperationUrl();

  if (!bodyMap.isEmpty()) {
    ZJsonArray array = ZJsonFactory::MakeJsonArray(bodyMap);
    writeJsonString(url, array.dumpString());
  } else {
    writeJsonString(url, "[]");
  }
}

/*
void ZDvidWriter::writeMergeOperation(
    const std::string &dataName, const std::string &key,
    const QMap<uint64_t, uint64_t> &bodyMap)
{
  std::string url = ZDvidUrl(m_dvidTarget).getMergeOperationUrl(dataName);
  ZJsonArray array = ZJsonFactory::MakeJsonArray(bodyMap);
  if (!array.isEmpty()) {
    writeJsonString(url, key, array.toString());
  }
}
*/
