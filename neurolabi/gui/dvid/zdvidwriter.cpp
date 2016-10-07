#include "zdvidwriter.h"
#include <iostream>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QElapsedTimer>
#include <QUrl>

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
#include "flyem/zflyembookmark.h"
#include "neutube.h"
#include "dvid/libdvidheader.h"
#include "flyem/zflyemtodoitem.h"
#include "zarray.h"
#include "flyem/zflyemmisc.h"
#include "dvid/zdvidbufferreader.h"
#include "zdvidutil.h"

ZDvidWriter::ZDvidWriter(QObject *parent) :
  QObject(parent)
{
  init();
//  m_eventLoop = new QEventLoop(this);
//  m_dvidClient = new ZDvidClient(this);
//  m_timer = new QTimer(this);
}

ZDvidWriter::~ZDvidWriter()
{
}

void ZDvidWriter::init()
{
  m_statusCode = 0;
}

bool ZDvidWriter::startService()
{
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    m_service = ZDvid::MakeDvidNodeService(m_dvidTarget);
  } catch (std::exception &e) {
    m_service.reset();
    std::cout << e.what() << std::endl;
    return false;
  }
#endif

  return true;
}

bool ZDvidWriter::open(
    const QString &serverAddress, const QString &uuid, int port)
{
//  m_dvidClient->reset();
  ZDvidTarget target;
  target.set(serverAddress.toStdString(), uuid.toStdString(), port);

  return open(target);
}

bool ZDvidWriter::open(const ZDvidTarget &target)
{
  if (!target.isValid()) {
    return false;
  }

  m_dvidTarget = target;

  std::string masterNode = ZDvidReader::ReadMasterNode(m_dvidTarget);
  if (!masterNode.empty()) {
    m_dvidTarget.setUuid(masterNode.substr(0, 4));
  }

//  m_dvidClient->reset();
//  m_dvidClient->setDvidTarget(target);

  return startService();;
}

void ZDvidWriter::clear()
{
  m_dvidTarget.clear();
}

bool ZDvidWriter::good() const
{
#if defined(_ENABLE_LIBDVIDCPP_)
  return m_service != NULL;
#else
  return m_dvidTarget.isValid();
#endif
}

bool ZDvidWriter::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString.toStdString());
  return open(target);
}

void ZDvidWriter::writeSwc(uint64_t bodyId, ZSwcTree *tree)
{
  if (tree != NULL) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    post(dvidUrl.getSkeletonUrl(bodyId), tree->toString(), false);
#if 0
    QString tmpPath = QString("%1/%2.swc").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(bodyId);
    tree->save(tmpPath.toStdString());

    ZDvidUrl dvidUrl(m_dvidTarget);
    QString command = QString("curl -i -X POST %1 --data-binary @%2").
        arg(dvidUrl.getSkeletonUrl(
              bodyId, m_dvidTarget.getBodyLabelName()).c_str()).arg(tmpPath);
    /*
    QString command = QString(
          "curl -X POST %1/api/node/%2/skeletons/%3.swc"
          " --data-binary @%4").arg(m_dvidClient->getServer()).
        arg(m_dvidClient->getUuid()).
        arg(bodyId).arg(tmpPath);
        */



    runCommand(command);
#endif

//    QProcess::execute(command);
  }
}

void ZDvidWriter::writeThumbnail(uint64_t bodyId, ZStack *stack)
{
  if (stack != NULL) {
    size_t length;
    char* buffer = C_Stack::toMrawBuffer(stack->data(), &length);
    ZDvidUrl dvidUrl(m_dvidTarget);
    post(dvidUrl.getThumbnailUrl(bodyId), buffer, length, false);
    free(buffer);
#if 0
    QString tmpPath = QString("%1/%2.mraw").
        arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
        arg(bodyId);
    stack->save(tmpPath.toStdString());
    ZDvidUrl dvidUrl(m_dvidTarget);

    QString command = QString("curl -i -X POST %1 --data-binary @%2").
        arg(dvidUrl.getThumbnailUrl(
              bodyId, m_dvidTarget.getBodyLabelName()).c_str()).
        arg(tmpPath);

    runCommand(command);
#endif
    /*
    qDebug() << command;

    QProcess::execute(command);
    */
  }
}

void ZDvidWriter::writeThumbnail(uint64_t bodyId, Stack *stack)
{
  if (stack != NULL) {
    Mc_Stack mstack;
    C_Stack::view(stack, &mstack);
    ZStack stackObj;
    stackObj.setData(&mstack, NULL);

    writeThumbnail(bodyId, &stackObj);
  }
}

void ZDvidWriter::writeAnnotation(uint64_t bodyId, const ZJsonObject &obj)
{
  if (bodyId > 0 && !obj.isEmpty()) {
    std::string url = ZDvidUrl(m_dvidTarget).getBodyAnnotationUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
#if defined(_ENABLE_LIBDVIDCPP_)
    post(url, obj);
#else
    QString command = QString(
          "curl -i -X POST -H \"Content-Type: application/json\" "
          "-d \"%1\" %2").arg(getJsonStringForCurl(obj).c_str()).
        arg(url.c_str());

    runCommand(command);
#endif
  }
}

void ZDvidWriter::writeAnnotation(const ZFlyEmNeuron &neuron)
{
  writeAnnotation(neuron.getId(), neuron.getAnnotationJson());
}

void ZDvidWriter::writeBodyAnntation(const ZFlyEmBodyAnnotation &annotation)
{
  writeAnnotation(annotation.getBodyId(), annotation.toJsonObject());
}

void ZDvidWriter::removeBodyAnnotation(uint64_t bodyId)
{
  ZDvidUrl url(m_dvidTarget);
  deleteKey(url.getBodyAnnotationName(), ZString::num2str(bodyId));
}

void ZDvidWriter::writeBodyInfo(uint64_t bodyId, const ZJsonObject &obj)
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

void ZDvidWriter::writeUrl(const std::string &url, const std::string &method)
{
  /*
  if (method == "POST") {
    post(url);
  } else if (method == "PUT") {
    put(url);
  } else {
  */
  QString command = QString("curl -i -X %1 %2").arg(method.c_str()).
      arg(url.c_str());

  runCommand(command);
//  }
}

void ZDvidWriter::writeJsonString(
    const std::string &url, const std::string &jsonString)
{
  post(url, jsonString, true);
#if 0
  QString annotationString = jsonString.c_str();

  QString command;
  if (annotationString.size() < 5000) {
//    annotationString.replace("\n", "");
//    annotationString.replace("\r", "");
    annotationString.replace("\"", "\"\"\"");

    command = QString(
          "curl -i -X POST -H \"Content-Type: application/json\" "
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

    command = QString("curl -i -X POST -H \"Content-Type: application/json\" "
                      "-d \"@%1\" %2").arg(tmpPath).arg(url.c_str());
  }

//  qDebug() << command;

//  QProcess::execute(command);
  runCommand(command);

//  qDebug() << getStandardOutput();
#endif
}


void ZDvidWriter::writeJson(const std::string &url, const ZJsonValue &value,
                            const std::string &emptyValueString)
{
  if (value.isEmpty()) {
    if (!emptyValueString.empty()) {
      writeJsonString(url, emptyValueString);
    }
  } else {
    writeJsonString(url, value.dumpString(0));
  }
}


void ZDvidWriter::mergeBody(const std::string &dataName,
                            uint64_t targetId,
                            const std::vector<uint64_t> &bodyId)
{
  ZJsonArray jsonArray(json_array(), ZJsonValue::SET_AS_IT_IS);
  jsonArray.append(targetId);
  for (std::vector<uint64_t>::const_iterator iter = bodyId.begin();
       iter != bodyId.end(); ++iter) {
    jsonArray.append(*iter);
  }

  /*
  ZJsonArray mergeArray(json_array(), ZJsonValue::SET_AS_IT_IS);
  mergeArray.append(jsonArray);
*/
  ZDvidUrl dvidUrl(m_dvidTarget);
  writeJson(dvidUrl.getMergeUrl(dataName), jsonArray, "[]");
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

  std::string url = ZDvidUrl(m_dvidTarget).getBoundBoxUrl(z);
  writeJson(url, obj);
#if 0


  //ZString annotationString = obj.dumpString(0);
  //annotationString.replace(" ", "");
  //annotationString.replace("\"", "\"\"\"");

  QString command = QString(
        "curl -i -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2").arg(getJsonStringForCurl(obj).c_str()).
      arg(ZDvidUrl(m_dvidTarget).getBoundBoxUrl(z).c_str());

  runCommand(command);
#endif
  /*
  qDebug() << command;

  QProcess::execute(command);
  */
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
//  jsonString.replace(" ", "");
  jsonString.replace("\"", "\"\"\"");

  return jsonString;
}

void ZDvidWriter::syncAnnotation(const std::string &name)
{
  ZDvidUrl url(getDvidTarget());
  ZJsonObject jsonObj;
  jsonObj.setEntry("sync", getDvidTarget().getLabelBlockName() + "," +
                   getDvidTarget().getBodyLabelName());
  post(url.getAnnotationSyncUrl(name), jsonObj);
}

void ZDvidWriter::syncLabelsz(
    const std::string &dataName, const std::string &annotationName)
{
  ZDvidUrl url(getDvidTarget());
  ZJsonObject jsonObj;
  jsonObj.setEntry("sync", annotationName);
  post(url.getLabelszSyncUrl(dataName), jsonObj);
}

void ZDvidWriter::syncSynapseLabelsz()
{
  syncLabelsz(getDvidTarget().getSynapseLabelszName(),
              getDvidTarget().getSynapseName());
}

void ZDvidWriter::createSynapseLabelsz()
{
  std::string dataName = getDvidTarget().getSynapseLabelszName();
  if (!dataName.empty()) {
    createData("labelsz", dataName);
    syncSynapseLabelsz();
  }
}


void ZDvidWriter::createData(
    const std::string &type, const std::string &name, bool versioned)
{
  if (type.empty() || name.empty()) {
    return;
  }

  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject obj;
  obj.setEntry("typename", type);
  obj.setEntry("dataname", name);
  if (!versioned) {
    obj.setEntry("versioned", "0");
  }

//  writeJson(dvidUrl.getInstanceUrl(), obj);


  std::string url = dvidUrl.getInstanceUrl();

#if 0
  post(url, obj);

  std::cout << obj.dumpString(2) << std::endl;
#endif

#if 1
  QString command = QString(
        "curl -i -X POST -H \"Content-Type: application/json\" -d \"%1\" %2").
      arg(getJsonStringForCurl(obj).c_str()).
      arg(url.c_str());
  /*
  qDebug() << command;

  QProcess::execute(command);
  */


  runCommand(command);
#endif
  if (type == "annotation") {
    syncAnnotation(name);
  }
}

void ZDvidWriter::deleteKey(const char* dataName, const char* key)
{
  if (strlen(dataName) == 0 || strlen(key) == 0) {
    return;
  }

  ZDvidUrl dvidUrl(m_dvidTarget);
  std::string url = dvidUrl.getKeyUrl(dataName, key);
  del(url);

}

void ZDvidWriter::deleteKey(const std::string &dataName, const std::string &key)
{
  if (dataName.empty() || key.empty()) {
    return;
  }

  deleteKey(dataName.c_str(), key.c_str());
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

void ZDvidWriter::deleteSkeleton(uint64_t bodyId)
{
  deleteKey(getDvidTarget().getSkeletonName(),
            ZDvidUrl::GetSkeletonKey(bodyId));
}

void ZDvidWriter::deleteBodyAnnotation(uint64_t bodyId)
{
  ZDvidUrl url(getDvidTarget());
  del(url.getBodyAnnotationUrl(bodyId));
}

void ZDvidWriter::invalidateBody(uint64_t bodyId)
{
  deleteBodyAnnotation(bodyId);
  deleteSkeleton(bodyId);
}

void ZDvidWriter::writeBodyInfo(uint64_t bodyId)
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
                  bodyId, m_dvidTarget.getBodyLabelName()), obj, "{}");
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

  post(ZDvidUrl(m_dvidTarget).getLockUrl(), messageJson.dumpString(0), true);

  return getStatusCode() == 200;

#if 0
  QString command = QString(
        "curl -i -X POST -H \"Content-Type: application/json\" "
        "-d \"%1\" %2").arg(getJsonStringForCurl(messageJson).c_str()).
      arg(ZDvidUrl(m_dvidTarget).getLockUrl().c_str());

  /*
  qDebug() << command;

  QProcess::execute(command);
  */

  return runCommand(command);
#endif
//  return true;
}

std::string ZDvidWriter::createBranch()
{
  std::string uuid;

  std::string response = post(ZDvidUrl(m_dvidTarget).getBranchUrl());

  if (getStatusCode() == 200) {
    ZJsonObject obj;
    obj.decodeString(response.c_str());

    if (obj.hasKey("child")) {
      uuid = ZJsonParser::stringValue(obj["child"]);
    }
  }
#if 0
  QProcess process;

  QString command = QString("curl -i -X POST %21").
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
#endif

  return uuid;
}

bool ZDvidWriter::runCommand(const QString &command, const QStringList &argList)
{
  QProcess process;
  process.start(command, argList);

  return runCommand(process);
}

bool ZDvidWriter::runCommand(const QString &command)
{
  std::cout << command.toStdString() << std::endl;
  if (command.length() <= 200) {
    LINFO() << command;
  } else {
    LINFO() << command.left(200) << "...";
  }
//  qDebug() << command;

  QProcess process;
  process.start(command);

  return runCommand(process);
}

bool ZDvidWriter::runCommand(QProcess &process)
{
  bool succ = process.waitForFinished(-1);

  m_errorOutput = process.readAllStandardError();
  m_standardOutout = process.readAllStandardOutput();

  parseStandardOutput();

  return succ;
}

#if defined(_ENABLE_LIBDVIDCPP_)
static libdvid::BinaryDataPtr makeRequest(
    const std::string &url, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode)
{
  libdvid::ConnectionMethod connMethod = libdvid::GET;
  if (method == "HEAD") {
    connMethod = libdvid::HEAD;
  } else if (method == "POST") {
    connMethod = libdvid::POST;
  } else if (method == "PUT") {
    connMethod = libdvid::PUT;
  } else if (method == "DELETE") {
    connMethod = libdvid::DELETE;
  }

  QUrl qurl(url.c_str());
  ZString address = qurl.host();
  if (qurl.port() >= 0) {
    address += ":";
    address.appendNumber(qurl.port());
  }
  libdvid::DVIDConnection connection(
        address, GET_FLYEM_CONFIG.getUserName(),
        NeutubeConfig::GetSoftwareName());

  libdvid::BinaryDataPtr results = libdvid::BinaryData::create_binary_data();
  std::string error_msg;

//  qDebug() << "address: " << address;
//  qDebug() << "path: " << qurl.path();

//  /*
//  statusCode = connection.make_request(
//        "/repo/372c/info", libdvid::GET, libdvid::BinaryDataPtr(),
//        results, error_msg, libdvid::DEFAULT);
//*/

  statusCode = connection.make_request(
        "/.." + qurl.path().toStdString(), connMethod, payload, results,
        error_msg, type);

//#if 0
//  statusCode = connection.make_request("/.." + qurl.path().toStdString(),
//        /*"/../api/node/372c/skeletons/key/1_swc",*/ libdvid::GET, payload, results,
//        error_msg, libdvid::BINARY);
//#endif

  return results;
}
#endif

std::string ZDvidWriter::request(
    const std::string &url, const std::string &method, const char *payload,
    int length, bool isJson)
{
  LINFO() << "HTTP " + method + ": " + url;

  m_statusCode = 0;
  std::string response;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    libdvid::BinaryDataPtr libdvidPayload;
    if (payload != NULL && length > 0) {
      libdvidPayload =
          libdvid::BinaryData::create_binary_data(payload, length);
    }

    libdvid::ConnectionMethod connMethod = libdvid::POST;
    if (method == "POST") {
      connMethod = libdvid::POST;
    } else if (method == "PUT") {
      connMethod = libdvid::PUT;
    } else if (method == "DELETE") {
      connMethod = libdvid::DELETE;
    }

    libdvid::BinaryDataPtr data;
    bool requested = false;
    if (m_service != NULL) {
      std::string endPoint = ZDvidUrl::GetEndPoint(url);

      if (!endPoint.empty()) {
        //    std::cout << libdvidPayload->get_data().size() << std::endl;
        requested = true;
        data = m_service->custom_request(endPoint, libdvidPayload, connMethod);

        m_statusCode = 200;
      }
    }

    if (!requested) {
      libdvid::ConnectionType type = libdvid::BINARY;
      if (isJson) {
        type = libdvid::JSON;
      }
      data = makeRequest(url, method, libdvidPayload, type, m_statusCode);
    }
    response = data->get_data();
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    LWARN() << "HTTP " + method + " exception (" << e.getStatus() << "): " << e.what();
    m_statusCode = e.getStatus();
  }
#endif

#ifdef _DEBUG_
  std::cout << response << std::endl;
#endif

  return response;
}


std::string ZDvidWriter::del(const std::string &url)
{
  return request(url, "DELETE", NULL, 0, false);

#if 0
  std::cout << "HTTP DELETE: " << url << std::endl;
  m_statusCode = 0;
  std::string response;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    libdvid::BinaryDataPtr libdvidPayload;
    libdvid::BinaryDataPtr data;

    std::string endPoint = ZDvidUrl::GetEndPoint(url);
    if (!endPoint.empty()) {
      //    std::cout << libdvidPayload->get_data().size() << std::endl;
      data = m_service->custom_request(
            endPoint, libdvidPayload, libdvid::DELETE);
      m_statusCode = 200;
    } else {
      data = makeRequest(url, "DELETE", libdvidPayload, m_statusCode);
    }
    response = data->get_data();

//    m_buffer.append(data->get_data().c_str(), data->length());
//    m_status = READ_OK;
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    LWARN() << "HTTP DELETE exception (" << e.getStatus() << "): " << e.what();
    m_statusCode = e.getStatus();
  }
#endif
  return response;
#endif
}

std::string ZDvidWriter::post(
    const std::string &url, const std::string &payload, bool isJson)
{
  return post(url, payload.c_str(), payload.length(), isJson);
}

std::string ZDvidWriter::post(
    const std::string &url, const char *payload, int length, bool isJson)
{
  return request(url, "POST", payload, length, isJson);
#if 0
  LINFO() << "HTTP POST: " << url;
  m_statusCode = 0;
  std::string response;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    std::string endPoint = ZDvidUrl::GetEndPoint(url);
    libdvid::BinaryDataPtr libdvidPayload;
    if (payload != NULL && length > 0) {
      libdvidPayload =
        libdvid::BinaryData::create_binary_data(payload, length);
    }
//    std::cout << libdvidPayload->get_data().size() << std::endl;
    libdvid::BinaryDataPtr data = m_service->custom_request(
          endPoint, libdvidPayload, libdvid::POST);

    response = data->get_data();
    m_statusCode = 200;
//    m_buffer.append(data->get_data().c_str(), data->length());
//    m_status = READ_OK;
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    LWARN() << "HTTP POST exception (" << e.getStatus() << "): " << e.what();
    m_statusCode = e.getStatus();
  }
#endif

  return response;
#endif
}

std::string ZDvidWriter::put(
    const std::string &url, const char *payload, int length, bool isJson)
{
  return request(url, "PUT", payload, length, isJson);
#if 0
  LINFO() << "HTTP PUT: " << url;
  m_statusCode = 0;
  std::string response;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    libdvid::BinaryDataPtr libdvidPayload;
    if (payload != NULL && length > 0) {
      libdvidPayload =
          libdvid::BinaryData::create_binary_data(payload, length);
    }

    std::string endPoint = ZDvidUrl::GetEndPoint(url);
    libdvid::BinaryDataPtr data;
    if (!endPoint.empty()) {
      //    std::cout << libdvidPayload->get_data().size() << std::endl;
      data = m_service->custom_request(
            endPoint, libdvidPayload, libdvid::PUT);

      m_statusCode = 200;
    } else {
      data = makeRequest(url, "PUT", libdvidPayload, isJson, m_statusCode);
    }
    response = data->get_data();
//    m_buffer.append(data->get_data().c_str(), data->length());
//    m_status = READ_OK;
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    LWARN() << "HTTP PUT exception (" << e.getStatus() << "): " << e.what();
    m_statusCode = e.getStatus();
  }
#endif

  return response;
#endif
}

std::string ZDvidWriter::post(const std::string &url)
{
  return post(url, NULL, 0, false);
}

std::string ZDvidWriter::put(const std::string &url)
{
  return put(url, NULL, 0, false);
}

std::string ZDvidWriter::post(const std::string &url, const QByteArray &payload,
                              bool isJson)
{
  return post(url, payload.constData(), payload.length(), isJson);
}

std::string ZDvidWriter::post(const std::string &url, const ZJsonObject &obj)
{
  std::string payload = obj.dumpString(0);
  return post(url, payload.c_str(), payload.size(), true);
}

std::string ZDvidWriter::post(const std::string &url, const ZJsonArray &obj)
{
  std::string payload = obj.dumpString(0);
  return post(url, payload.c_str(), payload.size(), true);
}

uint64_t ZDvidWriter::writeSplit(
    const ZObject3dScan &obj, uint64_t oldLabel, uint64_t label,
    uint64_t newBodyId)
{
  return writeSplit(
        m_dvidTarget.getBodyLabelName(), obj, oldLabel, label, newBodyId);
}

uint64_t ZDvidWriter::writeSplit(
    const std::string &dataName, const ZObject3dScan &obj,
    uint64_t oldLabel, uint64_t label, uint64_t newBodyId)
{
//  uint64_t newBodyId = 0;
  m_statusCode = 0;
#if defined(_ENABLE_LIBDVIDCPP_)
  UNUSED_PARAMETER(label);
  std::string url;
  if (newBodyId == 0) {
    url = ZDvidUrl(m_dvidTarget).getSplitUrl(dataName, oldLabel);
  } else {
    url = ZDvidUrl(m_dvidTarget).getSplitUrl(dataName, oldLabel, newBodyId);
  }

  QByteArray payload = obj.toDvidPayload();
  ZString response = post(url, payload, false);

  if (!response.empty()) {
#ifdef _DEBUG_
    std::cout << response << std::endl;
#endif
    ZJsonObject obj;
    obj.decodeString(response.c_str());
    if (obj.hasKey("label")) {
      newBodyId = ZJsonParser::integerValue(obj["label"]);
      m_statusCode = 200;
    } else {
      newBodyId = 0;
    }
  }
#else
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
        "curl -i -X POST %1 --data-binary \"@%2\"").
      arg(ZDvidUrl(m_dvidTarget).getSplitUrl(dataName, oldLabel).c_str()).
      arg(tmpPath);

//  qDebug() << command;

  if (runCommand(command)) {
    if (m_jsonOutput.hasKey("label")) {
      newBodyId = ZJsonParser::integerValue(m_jsonOutput["label"]);
    }
  }
#endif

  return newBodyId;
}

uint64_t ZDvidWriter::writeSplitMultires(const ZObject3dScan &bf,
    const ZObject3dScan &bs, uint64_t oldLabel)
{
  uint64_t newBodyId = 0;

  QElapsedTimer timer;
  timer.start();

#if defined(_ENABLE_LIBDVIDCPP_)
  if (bs.getVoxelNumber() >= 100000) {
    ZDvidInfo dvidInfo;
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      dvidInfo = reader.readGrayScaleInfo();
    } else {
      LERROR() << "DVID connection error.";
      return 0;
    }

    ZObject3dScan Bbf = dvidInfo.getBlockIndex(bf);

    ZObject3dScan bf_bs = bf;
    bf_bs.subtractSliently(bs);

    ZObject3dScan Bbf_bs = dvidInfo.getBlockIndex(bf_bs);

    ZObject3dScan Bsc = Bbf;
    Bsc.subtractSliently(Bbf_bs);

    std::cout << "Subract time: " << timer.elapsed() << std::endl;

    //Upload Bsc
    if (!Bsc.isEmpty()) {
      newBodyId = writeCoarseSplit(Bsc, oldLabel);

      std::cout << "Coarse time: " << timer.elapsed() << std::endl;
      if (newBodyId == 0) {
        LERROR() << "Failed to write coarse split.";
        return 0;
      }

      ZObject3dScan bBsc = Bsc;
      bBsc.translate(-dvidInfo.getStartBlockIndex());
      bBsc.upSample(dvidInfo.getBlockSize().getX() - 1,
                    dvidInfo.getBlockSize().getY() - 1,
                    dvidInfo.getBlockSize().getZ() - 1);
      bBsc.translate(dvidInfo.getStartCoordinates());

      ZObject3dScan bsr = bs;
      bsr.subtractSliently(bBsc);

      std::cout << "Coarse processing time: " << timer.elapsed() << std::endl;

      //Upload remaining part
      if (!bsr.isEmpty()) {
        writeSplit(bsr, oldLabel, 0, newBodyId);
        std::cout << "Fine time: " << timer.elapsed() << std::endl;
      }
    } else {
      newBodyId = writeSplit(bs, oldLabel, 0);
    }
  } else {
    newBodyId = writeSplit(bs, oldLabel, 0);
  }
#else
  UNUSED_PARAMETER(&bf);
  newBodyId = writeSplit(bs, oldLabel, 0);
#endif

  return newBodyId;
}

uint64_t ZDvidWriter::writePartition(
    const ZObject3dScan &bm, const ZObject3dScan &bs, uint64_t oldLabel)
{
  uint64_t newBodyId = 0;

  QElapsedTimer timer;
  timer.start();

#if defined(_ENABLE_LIBDVIDCPP_)
#ifdef _DEBUG_
  bm.exportDvidObject(GET_TMP_DIR + "/test_bm.dvid");
  bs.exportDvidObject(GET_TMP_DIR + "/test_bs.dvid");
#endif

  if (bs.getVoxelNumber() >= 100000) {
    ZDvidInfo dvidInfo;
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      dvidInfo = reader.readGrayScaleInfo();
    } else {
      LERROR() << "DVID connection error.";
      return 0;
    }

    ZObject3dScan Bsc = dvidInfo.getBlockIndex(bs);
    ZObject3dScan Bbf_bs = dvidInfo.getBlockIndex(bm);

//    ZObject3dScan Bsc = Bbf;
    Bsc.subtractSliently(Bbf_bs);

    std::cout << "Subract time: " << timer.elapsed() << std::endl;

    //Upload Bsc
    if (!Bsc.isEmpty()) {
      newBodyId = writeCoarseSplit(Bsc, oldLabel);

#ifdef _DEBUG_
      Bsc.exportDvidObject(GET_TMP_DIR + "/test.dvid");
#endif

      std::cout << "Coarse time: " << timer.elapsed() << std::endl;
//      newBodyId = 0;//debugging
      if (newBodyId == 0) {
        QString tmpPath = QString("%1/%2_Bsc.dvid").
            arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
            arg(oldLabel);
        LINFO() << "Saving" << tmpPath << "for debugging.";
        Bsc.exportDvidObject(tmpPath.toStdString());

        tmpPath = QString("%1/%2_bm.dvid").
            arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
            arg(oldLabel);
        LINFO() << "Saving" << tmpPath << "for debugging.";
        bm.exportDvidObject(tmpPath.toStdString());

        LERROR() << "Failed to write coarse split.";
        return 0;
      }

      ZObject3dScan Bbs = dvidInfo.getBlockIndex(bs);
      Bbs.subtractSliently(Bsc);
      ZObject3dScan bBbs = Bbs;
      bBbs.translate(-dvidInfo.getStartBlockIndex());
      bBbs.upSample(dvidInfo.getBlockSize().getX() - 1,
                    dvidInfo.getBlockSize().getY() - 1,
                    dvidInfo.getBlockSize().getZ() - 1);
      bBbs.translate(dvidInfo.getStartCoordinates());

      ZObject3dScan bsr = bs.intersect(bBbs);
#if 0
      ZObject3dScan bBsc = Bsc;
      bBsc.translate(-dvidInfo.getStartBlockIndex());
      bBsc.upSample(dvidInfo.getBlockSize().getX() - 1,
                    dvidInfo.getBlockSize().getY() - 1,
                    dvidInfo.getBlockSize().getZ() - 1);
      bBsc.translate(dvidInfo.getStartCoordinates());

      ZObject3dScan bsr = bs;
      bsr.subtractSliently(bBsc);
#endif

      std::cout << "Coarse processing time: " << timer.elapsed() << std::endl;

#ifdef _DEBUG_

#endif


      //Upload remaining part
      if (!bsr.isEmpty()) {
        newBodyId = writeSplit(bsr, oldLabel, 0, newBodyId);
        std::cout << "Fine time: " << timer.elapsed() << std::endl;
      }
    } else {
      newBodyId = writeSplit(bs, oldLabel, 0);
    }
  } else {
    newBodyId = writeSplit(bs, oldLabel, 0);
  }
#else
  UNUSED_PARAMETER(&bm);
  newBodyId = writeSplit(bs, oldLabel, 0);
#endif

  return newBodyId;
}

uint64_t ZDvidWriter::writeCoarseSplit(const ZObject3dScan &obj, uint64_t oldLabel)
{
  m_statusCode = 0;
  uint64_t newBodyId= 0;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    std::string url;
    if (newBodyId == 0) {
      url = ZDvidUrl(m_dvidTarget).getCoarseSplitUrl(
            m_dvidTarget.getBodyLabelName(), oldLabel);
    } else {
//      url = ZDvidUrl(m_dvidTarget).getSplitUrl(dataName, oldLabel, newBodyId);
    }

    QByteArray payload = obj.toDvidPayload();
    ZString response = post(url, payload, false);

    if (!response.empty()) {
#ifdef _DEBUG_
      std::cout << response << std::endl;
#endif
      ZJsonObject obj;
      obj.decodeString(response.c_str());
      if (obj.hasKey("label")) {
        newBodyId = ZJsonParser::integerValue(obj["label"]);
        m_statusCode = 200;
      }
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
#else
  QString tmpPath = QString("%1/%2_coarse.dvid").
      //arg((GET_TEST_DATA_DIR + "/backup").c_str()).
      arg(NeutubeConfig::getInstance().getPath(NeutubeConfig::TMP_DATA).c_str()).
      arg(oldLabel);

  obj.exportDvidObject(tmpPath.toStdString());

#ifdef _DEBUG_
  std::cout << tmpPath.toStdString() + " saved" << std::endl;
#endif

  QString command = QString(
        "curl -i -X POST %1 --data-binary \"@%2\"").
      arg(ZDvidUrl(m_dvidTarget).getCoarseSplitUrl(
            m_dvidTarget.getBodyLabelName(), oldLabel).c_str()).
      arg(tmpPath);

//  qDebug() << command;

  if (runCommand(command)) {
    if (m_jsonOutput.hasKey("label")) {
      newBodyId = ZJsonParser::integerValue(m_jsonOutput["label"]);
    }
  }
#endif

  return newBodyId;
}

void ZDvidWriter::writeMergeOperation(const QMap<uint64_t, uint64_t> &bodyMap)
{
  std::string url = ZDvidUrl(m_dvidTarget).getMergeOperationUrl(
        NeuTube::GetCurrentUserName());

  if (!url.empty()) {
    if (!bodyMap.isEmpty()) {
      ZJsonArray array = ZJsonFactory::MakeJsonArray(bodyMap);
      writeJsonString(url, array.dumpString(0));
    } else {
      writeJsonString(url, "[]");
    }
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

void ZDvidWriter::parseStandardOutput()
{
  m_statusCode = 0;
  m_jsonOutput.clear();

  if (!m_standardOutout.isEmpty()) {
    QStringList output =
        m_standardOutout.split(QRegExp("[\n\r]"), QString::SkipEmptyParts);
    qDebug() << output.back();

    foreach (QString str, output) {
      if (str.startsWith("HTTP/1.1 ")) {
        str.remove("HTTP/1.1 ");
        m_statusCode = ZString::firstInteger(str.toStdString());
      }
    }

    if (output.back().startsWith("{")) {
      m_jsonOutput.decodeString(output.back().toStdString().c_str());
    }

#ifdef _DEBUG_
    qDebug() << "Status code: " << m_statusCode;
    qDebug() << "Json output: " << m_jsonOutput.dumpString(2);
#endif
  }
}

void ZDvidWriter::writeBookmark(const ZFlyEmBookmark &bookmark)
{
  writePointAnnotation(
        m_dvidTarget.getBookmarkName(), bookmark.toDvidAnnotationJson());
//  writeBookmarkKey(bookmark);

  /*
  writeJsonString(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY),
                  bookmark.getDvidKey().toStdString(),
                  bookmark.toJsonObject().dumpString(0));
                  */
}

void ZDvidWriter::writeBookmarkKey(const ZFlyEmBookmark &bookmark)
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonObject json;
  if (bookmark.isChecked()) {
    json.setEntry("checked", true);
  }

  writeJson(dvidUrl.getBookmarkKeyUrl(bookmark.getCenter().toIntPoint()),
            json, "{}");
}

void ZDvidWriter::deleteBookmarkKey(const ZFlyEmBookmark &bookmark)
{
  ZIntPoint center = bookmark.getCenter().toIntPoint();
  std::stringstream stream;
  stream << center.getX() << "_" << center.getY() << "_" << center.getZ();

  deleteKey(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY), stream.str());
}


void ZDvidWriter::writeBookmark(const ZJsonArray &bookmarkJson)
{
  writePointAnnotation(m_dvidTarget.getBookmarkName(), bookmarkJson);
}

void ZDvidWriter::writeBookmark(const ZJsonObject &bookmarkJson)
{
  writePointAnnotation(m_dvidTarget.getBookmarkName(), bookmarkJson);
}

void ZDvidWriter::writeBookmark(
    const std::vector<ZFlyEmBookmark *> &bookmarkArray)
{
  if (!bookmarkArray.empty()) {
    ZJsonArray jsonArray = ZJsonFactory::MakeJsonArray(bookmarkArray);
    writePointAnnotation(m_dvidTarget.getBookmarkName(), jsonArray);
    /*
    for (std::vector<ZFlyEmBookmark*>::const_iterator
         iter = bookmarkArray.begin(); iter != bookmarkArray.end(); ++iter) {
      writeBookmarkKey(*(*iter));
    }
    */
  }
}

#if 0
void ZDvidWriter::writeCustomBookmark(const ZJsonValue &bookmarkJson)
{
  writeJson(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY),
            NeuTube::GetCurrentUserName(), bookmarkJson);
}

void ZDvidWriter::deleteAllCustomBookmark()
{
  writeCustomBookmark(ZJsonArray());
}
#endif

void ZDvidWriter::deletePointAnnotation(
    const std::string &dataName, int x, int y, int z)
{
#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvidUrl url(m_dvidTarget);
  del(url.getAnnotationUrl(dataName, x, y, z));
#else
  UNUSED_PARAMETER(&dataName);
  UNUSED_PARAMETER(x);
  UNUSED_PARAMETER(y);
  UNUSED_PARAMETER(z);
#endif
}

void ZDvidWriter::deletePointAnnotation(
    const std::string &dataName, const ZIntPoint &pt)
{
  deletePointAnnotation(dataName, pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidWriter::deleteBookmark(int x, int y, int z)
{
  deletePointAnnotation(m_dvidTarget.getBookmarkName(), x, y, z);
}

void ZDvidWriter::deleteBookmark(const ZIntPoint &pt)
{
  deleteBookmark(pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidWriter::deleteBookmark(
    const std::vector<ZFlyEmBookmark *> &bookmarkArray)
{
  for (std::vector<ZFlyEmBookmark *>::const_iterator
       iter = bookmarkArray.begin(); iter != bookmarkArray.end(); ++iter) {
    const ZFlyEmBookmark *bookmark = *iter;
    deleteBookmark(bookmark->getCenter().toIntPoint());
  }
}

void ZDvidWriter::deleteToDoItem(int x, int y, int z)
{
#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvidUrl url(m_dvidTarget);
  del(url.getTodoListDeleteUrl(x, y, z));
#else
  UNUSED_PARAMETER(x);
  UNUSED_PARAMETER(y);
  UNUSED_PARAMETER(z);
#endif
}

void ZDvidWriter::writeToDoItem(const ZFlyEmToDoItem &item)
{
  ZDvidUrl url(m_dvidTarget);
  ZJsonArray itemJson;
  itemJson.append(item.toJsonObject());

  writeJson(url.getTodlListElementsUrl(), itemJson);
}

void ZDvidWriter::writeLabel(const ZArray &label)
{
  if (!label.isEmpty()) {
    ZDvidUrl url(getDvidTarget());
    post(url.getLabels64Url(label.getDim(0), label.getDim(1), label.getDim(2),
                            label.getStartCoordinate(0),
                            label.getStartCoordinate(1),
                            label.getStartCoordinate(2)) + "?mutate=true",
         label.getDataPointer<char>(), label.getByteNumber(), false);
  }
}

void ZDvidWriter::refreshLabel(const ZIntCuboid &box, uint64_t bodyId)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    ZIntCuboid alignedBox;
    alignedBox.setFirstCorner(
          dvidInfo.getBlockBox(
            dvidInfo.getBlockIndex(box.getFirstCorner())).getFirstCorner());
    alignedBox.setLastCorner(
          dvidInfo.getBlockBox(
            dvidInfo.getBlockIndex(box.getLastCorner())).getLastCorner());

    ZArray *label = reader.readLabels64(alignedBox);

    //Reset label
    uint64_t tmpLabel = label->getMax<uint64_t>() + 1;
    label->replaceValue(bodyId, tmpLabel);

    writeLabel(*label);

    label->replaceValue(tmpLabel, bodyId);
    writeLabel(*label);

    delete label;
  }
}

void ZDvidWriter::refreshLabel(
    const ZIntCuboid &box, const std::set<uint64_t> &bodySet)
{
  if (!bodySet.empty()) {
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
      ZIntCuboid alignedBox;
      alignedBox.setFirstCorner(
            dvidInfo.getBlockBox(
              dvidInfo.getBlockIndex(box.getFirstCorner())).getFirstCorner());
      alignedBox.setLastCorner(
            dvidInfo.getBlockBox(
              dvidInfo.getBlockIndex(box.getLastCorner())).getLastCorner());

      ZArray *label = reader.readLabels64(alignedBox);

      if (reader.getStatusCode() == 200) {
        //Reset label
        uint64_t labelMax = label->getMax<uint64_t>();
        uint64_t tmpLabel = labelMax + 1;
        for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          uint64_t bodyId = *iter;
          label->replaceValue(bodyId, tmpLabel++);
        }

        writeLabel(*label);

        tmpLabel = labelMax + 1;
        for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          uint64_t bodyId = *iter;
          label->replaceValue(tmpLabel++, bodyId);
        }

        writeLabel(*label);
      }

      delete label;
    }
  }
}


void ZDvidWriter::deleteSynapse(int x, int y, int z)
{
#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvidUrl url(m_dvidTarget);
  del(url.getSynapseUrl(x, y, z));
#else
  UNUSED_PARAMETER(x);
  UNUSED_PARAMETER(y);
  UNUSED_PARAMETER(z);
#endif
}

void ZDvidWriter::writePointAnnotation(
    const std::string &dataName, const ZJsonObject &annotationJson)
{
  ZDvidUrl url(m_dvidTarget);
  ZJsonArray json;
  json.append(annotationJson);

  writePointAnnotation(dataName, json);
}

void ZDvidWriter::writePointAnnotation(
    const std::string &dataName, const ZJsonArray &annotationJson)
{
  ZDvidUrl url(m_dvidTarget);

  writeJson(url.getAnnotationElementsUrl(dataName), annotationJson);
}

void ZDvidWriter::writeSynapse(const ZDvidSynapse &synapse)
{
  ZDvidUrl url(m_dvidTarget);
  ZJsonArray synapseJson;
  synapseJson.append(synapse.toJsonObject());
#ifdef _DEBUG_
  std::cout << synapseJson.dumpString(2) << std::endl;
#endif
  writeJson(url.getSynapseElementsUrl(), synapseJson);
}

void ZDvidWriter::writeSynapse(const ZJsonObject &synapseJson)
{
  ZDvidUrl url(m_dvidTarget);
  ZJsonArray synapseArrayJson;
  synapseArrayJson.append(synapseJson);

  writeJson(url.getSynapseElementsUrl(), synapseArrayJson);
}

void ZDvidWriter::writeSynapse(const ZJsonArray &synapseJson)
{
  ZDvidUrl url(m_dvidTarget);

#ifdef _DEBUG_
  std::cout << synapseJson.dumpString(2) << std::endl;
#endif
  writeJson(url.getSynapseElementsUrl(), synapseJson);
}

void ZDvidWriter::moveSynapse(const ZIntPoint &from, const ZIntPoint &to)
{
#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvidUrl url(m_dvidTarget);
  post(url.getSynapseMoveUrl(from, to));
#else
  UNUSED_PARAMETER(&from);
  UNUSED_PARAMETER(&to);
#endif
}

void ZDvidWriter::addSynapseProperty(
    const ZIntPoint &synapse, const std::string &key, const std::string &value)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZJsonObject synapseJson = reader.readSynapseJson(synapse);
    if (!synapseJson.isEmpty()) {
      ZDvidSynapse::AddProperty(synapseJson, key, value);
      writeSynapse(synapseJson);
    }
  }
}

void ZDvidWriter::writeMasterNode(const std::string &uuid)
{
#if defined(_FLYEM_)
//  std::string rootNode =
//      GET_FLYEM_CONFIG.getDvidRootNode(getDvidTarget().getUuid());
//  if (!rootNode.empty()) {
    ZDvidBufferReader reader;
    ZDvidUrl dvidUrl(getDvidTarget());
    dvidUrl.setUuid(uuid);
    std::string url = dvidUrl.getApiUrl() + "/node/" + uuid +
        "/branches/key/master";
    LINFO() << "Master url: " << url;
    reader.read(url.c_str());
    ZJsonArray branchJson;
    branchJson.append(uuid);

    ZJsonArray oldBranchJson;
    oldBranchJson.decodeString(reader.getBuffer().data());
    for (size_t i = 0; i < oldBranchJson.size(); ++i) {
      if (ZJsonParser::stringValue(oldBranchJson.at(i)) != uuid) {
        branchJson.append(oldBranchJson.at(i));
      }
    }

    ZDvid::MakeRequest(
          url, "POST", ZDvid::MakePayload(branchJson), libdvid::JSON,
          m_statusCode);
#endif
//    ZFlyEmMisc::MakeRequest(url,
//    post(url, branchJson);
//  }
}
