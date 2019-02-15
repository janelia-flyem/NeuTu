#include "neuprintreader.h"

#include <QFile>

#include "logging/zqslog.h"
#include "logging/zlog.h"

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "cypherquery.h"

NeuPrintReader::NeuPrintReader(const QString &server) : m_server(server)
{
}

QString NeuPrintReader::getServer() const
{
  return m_server;
}

void NeuPrintReader::setServer(const QString &server)
{
  if (server != m_server) {
    m_server = server;
    m_dataset.clear();
    m_currentDataset.clear();
  }
}

void NeuPrintReader::authorize(const QString &token)
{
  if (!token.isEmpty()) {
    m_bufferReader.setRequestHeader("Authorization", QString("Bearer ") + token);
  }
}

void NeuPrintReader::authorizeFromJson(const QString &auth)
{
  if (!auth.isEmpty()) {
    ZJsonObject obj;
    obj.decode(auth.toStdString());
    std::string token = ZJsonParser::stringValue(obj["token"]);
    if (!token.empty()) {
      authorize(token.c_str());
    } else {
      LWARN() << "No NeuPrint token found.";
    }
  }
}

void NeuPrintReader::authorizeFromFile(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
      QTextStream stream(&f);
      qDebug() << "Auth: " << stream.readAll();
      authorizeFromJson(stream.readAll());
    }
  }
}

void NeuPrintReader::readDatasets()
{
  m_dataset.clear();
  m_bufferReader.read(m_server + "/api/dbmeta/datasets", true);

  if (m_bufferReader.getStatus() == neutu::EReadStatus::OK) {
    m_dataset.decode(m_bufferReader.getBuffer().toStdString());
  }

  if (m_dataset.isEmpty()) {
    LWARN() << "No datasets retreived from NeuPrint.";
  }
}

bool NeuPrintReader::hasAuthCode() const
{
  return m_bufferReader.hasRequestHeader("Authorization");
}

bool NeuPrintReader::isConnected()
{
  if (m_dataset.isEmpty()) {
    readDatasets();
  }

  return !m_dataset.isEmpty();
}

bool NeuPrintReader::connect()
{
  readDatasets();
  return !m_dataset.isEmpty();
}

bool NeuPrintReader::isReady()
{
  if (isConnected()) {
    return m_dataset.hasKey(m_currentDataset.toStdString());
  }

  return false;
}

QString NeuPrintReader::getNeuronLabel(char quote) const
{
  QString label =  m_currentDataset + "-Neuron";
  if (quote != '\0') {
    label = quote + label + quote;
  }

  return label;
}

QList<QString> NeuPrintReader::getRoiList()
{
  QList<QString> roiList;

  if (isReady()) {
    ZJsonObject dataObj(m_dataset.value(m_currentDataset.toStdString()));
    ZJsonArray roiJson(dataObj.value("ROIs"));
    for (size_t i = 0; i < roiJson.size(); ++i) {
      roiList.append(ZJsonParser::stringValue(roiJson.at(i)).c_str());
    }
  }

  return roiList;
}

QString NeuPrintReader::getUuidKey(const QString &uuid)
{
  QString uuidKey;
  if (isConnected()) {
    const char *key = nullptr;
    json_t *value = nullptr;
    ZJsonObject_foreach(m_dataset, key, value) {
      ZJsonObject dataObj(value, ZJsonValue::SET_INCREASE_REF_COUNT);
      if (dataObj.hasKey("uuid")) {
        QString fullUuid(ZJsonParser::stringValue(dataObj["uuid"]).c_str());
        if (fullUuid.startsWith(uuid)){
          uuidKey = key;
          break;
        }
      }
    }
  }

  return uuidKey;
}

void NeuPrintReader::updateCurrentDataset(const QString &uuid)
{
  m_currentDataset = getUuidKey(uuid);
}

bool NeuPrintReader::hasDataset(const QString &uuid)
{
  return !getUuidKey(uuid).isEmpty();
}

namespace {
//Assuming the following order: ID, name, status, pre, post
ZJsonArray extract_body_info(const QByteArray &response)
{
  ZJsonArray bodies;

  ZJsonObject resultObj;
  resultObj.decode(response.toStdString());

  if (resultObj.hasKey("data")) {
    ZJsonArray data(resultObj.value("data"));
    if (data.size() >= 5) {
      for (size_t i = 0; i < data.size(); ++i) {
        uint64_t bodyId = ZJsonParser::integerValue(data.at(i), 0);
        ZJsonObject bodyData;
        bodyData.setEntry("body ID", bodyId);
        std::string name = ZJsonParser::stringValue(data.at(i), 1);
        if (!name.empty()) {
          bodyData.setEntry("name", name);
        }
        std::string status = ZJsonParser::stringValue(data.at(i), 2);
        if (!status.empty()) {
          bodyData.setEntry("body status", status);
        }
        bodyData.setEntry("body T-bars", ZJsonParser::integerValue(data.at(i), 3));
        bodyData.setEntry("body PSDs", ZJsonParser::integerValue(data.at(i), 4));
        bodies.append(bodyData);
      }
    }
  }

  return bodies;
}

static QList<uint64_t> extract_body_list(const QByteArray &response)
{
  ZJsonObject resultObj;
  resultObj.decode(response.toStdString());

  resultObj.print();

  QList<uint64_t> bodyList;
  if (resultObj.hasKey("data")) {
    ZJsonArray data(resultObj.value("data"));
    for (size_t i = 0; i < data.size(); ++i) {
#ifdef _DEBUG_
      std::cout << "Body ID: " << ZJsonParser::integerValue(data.at(i), 0) << std::endl;
#endif
      uint64_t bodyId = ZJsonParser::integerValue(data.at(i), 0);
      bodyList.append(bodyId);
    }
  }

  return bodyList;
}

}

ZJsonArray NeuPrintReader::queryTopNeuron(int n)
{
  if (n > 0) {
    QString url = m_server + "/api/custom/custom";
    ZJsonObject dataObj;
    dataObj.setEntry("dataset", m_currentDataset.toStdString());

    CypherQuery query = CypherQueryBuilder().
        match(QString("(n:%1)").arg(getNeuronLabel('`'))).
        ret("n.bodyId, n.name, n.status, n.pre, n.post").
        orderDesc("(n.pre + n.post)").limit(n);
    QString queryString = query.getQueryString();

    dataObj.setEntry("cypher", queryString.toStdString());

#ifdef _DEBUG_
    std::cout << "Query:" << std::endl;
    dataObj.print();
#endif

    m_bufferReader.post(url, dataObj.dumpString(0).c_str());

    return extract_body_info(m_bufferReader.getBuffer());
  }

  return ZJsonArray();
}

ZJsonArray NeuPrintReader::findSimilarNeuron(const uint64_t bodyId)
{
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  dataObj.setEntry("bodyId", bodyId);

  CypherQuery query = CypherQueryBuilder().
      match("(m:Meta{dataset:'" + m_currentDataset + "'})").
      with("m.superLevelRois", "rois").
      match(QString("(n:%1{bodyId:%2})").arg(getNeuronLabel('`')).arg(bodyId)).
      with("n.clusterName AS cn, rois").
      match(QString("(n:%1{clusterName:cn})").arg(getNeuronLabel('`'))).
      ret("n.bodyId, n.name, n.status, n.pre, n.post");

#ifdef _DEBUG_2
  std::cout << "Query: " << query.getQueryString().toStdString() << std::endl;
#endif

  dataObj.setEntry("cypher", query.getQueryString().toStdString());

  /*
  dataObj.setEntry("cypher", "MATCH (m:Meta{dataset:'"
                             + m_currentDataset.toStdString() +
                             "'}) "
                             "WITH m.superLevelRois AS rois "
                             "MATCH (n:"
                             + getNeuronLabel('`').toStdString() +
                             "{bodyId:" +
                             std::to_string(bodyId) +
                             "}) "
                             "WITH n.clusterName AS cn, rois "
                             "MATCH (n:"
                             + getNeuronLabel('`').toStdString() +
                             "{clusterName:cn}) "
                             "RETURN n.bodyId, n.name, n.status, n.pre, n.post");
                             */

#ifdef _DEBUG_
  dataObj.print();
#endif

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

ZJsonArray NeuPrintReader::queryAllNamedNeuron()
{
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  QString query = "MATCH (n:"
      + getNeuronLabel('`') +
      ") "
      "WHERE exists(n.name) AND n.name =~ '.*[^\\\\*]' "
      "RETURN n.bodyId, n.name, n.status, n.pre, n.post";
  if (m_numberLimit > 0) {
     query += QString(" ORDER BY (n.pre + n.post) DESC LIMIT %1").arg(m_numberLimit);
  }
  dataObj.setEntry("cypher", query.toStdString());

#ifdef _DEBUG_
  std::cout << "Query:" << std::endl;
  dataObj.print();
#endif

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

ZJsonArray NeuPrintReader::queryNeuronByName(const QString &name)
{
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  QString query = "MATCH (n:"
      + getNeuronLabel('`') +
      "{name:\"" + name + "\"}) "
//      "WHERE n.name=\"" + name + "\" "
      "RETURN n.bodyId, n.name, n.status, n.pre, n.post";
  if (m_numberLimit > 0) {
     query += QString(" ORDER BY (n.pre + n.post) DESC LIMIT %1").arg(m_numberLimit);
  }
  dataObj.setEntry("cypher", query.toStdString());

  KINFO << query;

#ifdef _DEBUG_
  std::cout << "Query:" << std::endl;
  dataObj.print();
#endif

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

ZJsonArray NeuPrintReader::queryNeuronByStatus(const QString &status)
{
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());

  CypherQuery query = CypherQueryBuilder().
      match(QString("(n:%1)").arg(getNeuronLabel('`'))).
      where(QString("LOWER(n.status) = LOWER(\"%1\")").arg(status)).
      ret("n.bodyId, n.name, n.status, n.pre, n.post");
  QString queryString = query.getQueryString();

//  QString query = "MATCH (n:"
//      + getNeuronLabel('`') +
//      "{status:\"" + status + "\"}) "
//      "RETURN n.bodyId, n.name, n.status, n.pre, n.post";
  if (m_numberLimit > 0) {
     queryString += QString(" ORDER BY (n.pre + n.post) DESC LIMIT %1").arg(m_numberLimit);
  }
  dataObj.setEntry("cypher", queryString.toStdString());

  KINFO << queryString;

#ifdef _DEBUG_
  std::cout << "Query:" << std::endl;
  dataObj.print();
#endif

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

ZJsonObject NeuPrintReader::customQuery(const QString &query)
{
  ZJsonObject resultObj;

  if (!query.isEmpty()) {
    QString url = m_server + "/api/custom/custom";
    m_bufferReader.post(url, query.toStdString().c_str());

    resultObj.decode(m_bufferReader.getBuffer().toStdString());
  }

  return resultObj;
}

ZJsonObject NeuPrintReader::customQuery(const ZJsonObject &json)
{
  return customQuery(json.dumpString(0).c_str());
}

QList<uint64_t> NeuPrintReader::queryNeuron(
    const QList<QString> &inputRoiList, const QList<QString> &outputRoiList)
{
  QString url = m_server + "/api/npexplorer/findneurons";

  ZJsonObject dataObj;
  dataObj.setEntry("dataset", "hemibrain");
  ZJsonArray inputRoiJson;
  for (const QString inputRoi : inputRoiList) {
    inputRoiJson.append(inputRoi.toStdString());
  }
  dataObj.setEntry("input_ROIs", inputRoiJson);

  ZJsonArray outputRoiJson;
  for (const QString outputRoi : outputRoiList) {
    outputRoiJson.append(outputRoi.toStdString());
  }
  dataObj.setEntry("output_ROIs", outputRoiJson);

  dataObj.setEntry("pre_threshold", 2);

  std::string queryStr = dataObj.dumpString(0);
  m_bufferReader.post(url, queryStr.c_str());

  KINFO << queryStr;

  return extract_body_list(m_bufferReader.getBuffer());
}


