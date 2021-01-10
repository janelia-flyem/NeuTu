#include "neuprintreader.h"

#include <QFile>

#include "logging/zqslog.h"
#include "logging/zlog.h"

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "cypherquery.h"
#include "qt/core/utilities.h"

NeuPrintReader::NeuPrintReader(const QString &server) : m_server(server)
{
}

QString NeuPrintReader::getServer() const
{
  return m_server;
}

void NeuPrintReader::setServer(const QString &server)
{
  QString normalizedServer = neutu::NormalizeServerAddress(server);

  if (server != m_server) {
    m_server = server;
    m_dataset.clear();
    m_currentDataset.clear();
  }
}

void NeuPrintReader::authorize(const QString &token)
{
  if (m_token != token) {
    m_dataset.clear();

    m_token = token;
    if (!token.isEmpty()) {
      m_bufferReader.setRequestHeader("Authorization", QString("Bearer ") + token);
    } else {
      m_bufferReader.removeRequestHeader("Authorization");
    }
  }
}

void NeuPrintReader::authorizeFromJson(const QString &auth)
{
  if (!auth.isEmpty()) {
    ZJsonObject obj;
    if (obj.decode(auth.toStdString(), true)) {
      std::string token = ZJsonParser::stringValue(obj["token"]);
      if (!token.empty()) {
        authorize(token.c_str());
      } else {
        KWARN << "No NeuPrint token found.";
      }
    } else {
      LKWARN << "Invalid auth json: " + auth;
    }
  }
}

void NeuPrintReader::authorizeFromFile(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
      QTextStream stream(&f);
//      qDebug() << "Auth: " << stream.readAll();
      authorizeFromJson(stream.readAll());
    }
  }
}

QString NeuPrintReader::getToken() const
{
  return m_token;
}

void NeuPrintReader::readDatasets()
{
  m_dataset.clear();
  m_bufferReader.read(m_server + "/api/dbmeta/datasets", true);

  if (m_bufferReader.getStatus() == neutu::EReadStatus::OK) {
    m_dataset.decode(m_bufferReader.getBuffer().toStdString(), true);
    if (m_dataset.isEmpty()) {
      m_status = neutu::EServerStatus::NOSUPPORT;
    } else {
      m_status = neutu::EServerStatus::NORMAL;
    }
  } else {
    if (m_bufferReader.getStatusCode() == 401) { //unauthorized
      m_status = neutu::EServerStatus::NOAUTH;
    } else {
      m_status = neutu::EServerStatus::OFFLINE;
    }
  }

  if (m_dataset.isEmpty()) {
    KWARN << "No datasets retreived from NeuPrint: " +
             m_bufferReader.getBuffer().toStdString();
  }
}

bool NeuPrintReader::hasAuthCode() const
{
  return m_bufferReader.hasRequestHeader("Authorization");
}

neutu::EServerStatus NeuPrintReader::getStatus() const
{
  if (m_status == neutu::EServerStatus::NORMAL) {
    if (m_dataset.hasKey(m_currentDataset.toStdString()) == false) {
      return neutu::EServerStatus::NOSUPPORT;
    }
  }

   return m_status;
 }

bool NeuPrintReader::isConnected()
{
  if (m_dataset.isEmpty()) {
    readDatasets();
  }

  return (m_status == neutu::EServerStatus::NORMAL);
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
  QString prefix = m_currentDataset;
  if (prefix.contains(":")) {
    prefix = prefix.split(":")[0];
  }

  QString label =  prefix + "_Neuron";
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

void NeuPrintReader::updateCurrentDatasetFromUuid(const QString &uuid)
{
  m_currentDataset = getUuidKey(uuid);
}

bool NeuPrintReader::hasDataset(const QString &uuid)
{
  return !getUuidKey(uuid).isEmpty();
}

QString NeuPrintReader::getDataset(const QString &uuid)
{
  return getUuidKey(uuid);
}

void NeuPrintReader::setCurrentDataset(const QString &dataset)
{
  m_currentDataset = dataset;
}

ZJsonObject NeuPrintReader::getDatasetJson() const
{
  return m_dataset;
}

QStringList NeuPrintReader::getDatasetList() const
{
  QStringList dataList;
  std::vector<std::string> keys = getDatasetJson().getAllKey();
  for (const std::string &key : keys) {
    dataList.append(key.c_str());
  }

  return dataList;
}

QString NeuPrintReader::getCurrentDataset() const
{
  return m_currentDataset;
}

namespace {
const char* BODY_QUERY_RETURN =
    "n.bodyId, n.type, n.instance, n.status, n.pre, n.post, n.cellBodyFiber";
const char* BODY_QUERY_SYNAPSE_COUNT = "(n.pre + n.post)";

//Assuming the following order: ID, type name, status, pre, post
ZJsonArray extract_body_info(const QByteArray &response)
{
  ZJsonArray bodies;

  ZJsonObject resultObj;
  resultObj.decode(response.toStdString(), false);

  if (resultObj.hasKey("data")) {
    ZJsonArray data(resultObj.value("data"));
    for (size_t i = 0; i < data.size(); ++i) {
      ZJsonArray entry(data.value(i));

      if (entry.size() > 6) {
        size_t index = 0;
        uint64_t bodyId = uint64_t(
              ZJsonParser::integerValue(entry.getData(), index++));
        ZJsonObject bodyData;
        bodyData.setEntry("body ID", bodyId);
        std::string type = ZJsonParser::stringValue(entry.getData(), index++);
        if (!type.empty()) {
          bodyData.setEntry("class", type);
        }

        std::string name = ZJsonParser::stringValue(entry.getData(), index++);
        if (!name.empty()) {
          bodyData.setEntry("name", name);
        }
        std::string status = ZJsonParser::stringValue(entry.getData(), index++);
        if (!status.empty()) {
          bodyData.setEntry("body status", status);
        }

        bodyData.setEntry(
              "body T-bars", ZJsonParser::integerValue(entry.getData(), index++));
        bodyData.setEntry(
              "body PSDs", ZJsonParser::integerValue(entry.getData(), index++));

        std::string pn = ZJsonParser::stringValue(entry.getData(), index++);
        if (!pn.empty()) {
          bodyData.setEntry("primary neurite", pn);
        }
        bodies.append(bodyData);
      }
    }
  }

  return bodies;
}

QList<uint64_t> extract_body_list(const QByteArray &response)
{
  ZJsonObject resultObj;
  resultObj.decode(response.toStdString(), false);

//  resultObj.print();

  QList<uint64_t> bodyList;
  if (resultObj.hasKey("data")) {
    ZJsonArray data(resultObj.value("data"));
#ifdef _DEBUG_
    if (data.size() == 0) {
      KWARN << "No body found";
    }
#endif
    for (size_t i = 0; i < data.size(); ++i) {
#ifdef _DEBUG_
      std::cout << "Body ID: " << ZJsonParser::integerValue(data.at(i), 0) << std::endl;
#endif
      uint64_t bodyId = ZJsonParser::integerValue(data.at(i), 0);
      bodyList.append(bodyId);
    }
  } else {
    KWARN << "Unexpected response:"
          << QString::fromStdString(response.toStdString()).left(100);
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
        ret(BODY_QUERY_RETURN).
        orderDesc("(n.pre + n.post)").limit(n);
    QString queryString = query.getQueryString();

    dataObj.setEntry("cypher", queryString.toStdString());

    KINFO << "Query:" << dataObj.dumpString(0);
#ifdef _DEBUG_2
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
  if (isReady()) {
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
        ret(BODY_QUERY_RETURN);

#ifdef _DEBUG_2
    std::cout << "Query: " << query.getQueryString().toStdString() << std::endl;
#endif

    dataObj.setEntry("cypher", query.getQueryString().toStdString());


#ifdef _DEBUG_2
    dataObj.print();
#endif
    KINFO << "Query:" << dataObj.dumpString(0);

    m_bufferReader.post(url, dataObj.dumpString(0).c_str());

    return extract_body_info(m_bufferReader.getBuffer());
  }

  return ZJsonArray();
}

ZJsonArray NeuPrintReader::queryAllNamedNeuron()
{
  if (!isReady()) {
    return ZJsonArray();
  }

  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  QString query = "MATCH (n:"
      + getNeuronLabel('`') +
      ") "
      "WHERE exists(n.instance) AND n.instance =~ '.*[^\\\\*]' "
      "RETURN " + BODY_QUERY_RETURN;
  if (m_numberLimit > 0) {
     query += QString(" ORDER BY (n.pre + n.post) DESC LIMIT %1").arg(m_numberLimit);
  }
  dataObj.setEntry("cypher", query.toStdString());

#ifdef _DEBUG_2
  std::cout << "Query:" << std::endl;
  dataObj.print();
#endif
  KINFO << "Query:" << dataObj.dumpString(0);

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

QString NeuPrintReader::getCustomUrl() const
{
  return m_server + "/api/custom/custom";
}

ZJsonObject NeuPrintReader::getQueryJsonObject(const QString &query)
{
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  dataObj.setEntry("cypher", query.toStdString());

  return dataObj;
}

ZJsonArray NeuPrintReader::queryNeuron(const QString &query)
{
  if (!isReady()) {
    return ZJsonArray();
  }

  ZJsonObject dataObj = getQueryJsonObject(query);

  KINFO << "Query:" << dataObj.dumpString(0);

  m_bufferReader.post(getCustomUrl(), dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
}

ZJsonArray NeuPrintReader::queryNeuronByType(const QString &type)
{
  CypherQuery query = CypherQueryBuilder().
      match(QString("(n:%1 {type:\"%2\"})").arg(getNeuronLabel('`')).arg(type)).
      orderDesc(BODY_QUERY_SYNAPSE_COUNT).
      limit(m_numberLimit).
      ret(BODY_QUERY_RETURN);

  return queryNeuron(query.getQueryString());
}

ZJsonArray NeuPrintReader::queryNeuronByName(const QString &name)
{
  CypherQuery query = CypherQueryBuilder().
      match(QString("(n:%1 {instance:\"%2\"})").arg(getNeuronLabel('`')).arg(name)).
      orderDesc(BODY_QUERY_SYNAPSE_COUNT).
      limit(m_numberLimit).
      ret(BODY_QUERY_RETURN);

  return queryNeuron(query.getQueryString());

#if 0
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", m_currentDataset.toStdString());
  QString query = "MATCH (n:"
      + getNeuronLabel('`') +
      "{name:\"" + name + "\"}) "
//      "WHERE n.name=\"" + name + "\" "
      "RETURN " + BODY_QUERY_RETURN;
  if (m_numberLimit > 0) {
     query += QString(" ORDER BY (n.pre + n.post) DESC LIMIT %1").arg(m_numberLimit);
  }
  dataObj.setEntry("cypher", query.toStdString());

//  KINFO << query;

#ifdef _DEBUG_2
  std::cout << "Query:" << std::endl;
  dataObj.print();
#endif
  KINFO << "Query:" << dataObj.dumpString(0);

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_info(m_bufferReader.getBuffer());
#endif
}

ZJsonArray NeuPrintReader::queryNeuronByStatus(const QList<QString> &statusList)
{
  QList<QString> valueList = statusList;
  std::transform(valueList.begin(), valueList.end(), valueList.begin(),
                 [](const QString &str) { return "LOWER(\"" + str + "\")"; });

  CypherQuery query = CypherQueryBuilder().
      match(QString("(n:%1)").arg(getNeuronLabel('`'))).
      where(CypherQueryBuilder::OrEqualClause("LOWER(n.status)", valueList)).
      orderDesc(BODY_QUERY_SYNAPSE_COUNT).
      limit(m_numberLimit).
      ret(BODY_QUERY_RETURN);

  return queryNeuron(query.getQueryString());
}

ZJsonArray NeuPrintReader::queryNeuronByStatus(const QString &status)
{
  CypherQuery query = CypherQueryBuilder().
      match(QString("(n:%1)").arg(getNeuronLabel('`'))).
      where(QString("LOWER(n.status) = LOWER(\"%1\")").arg(status)).
      orderDesc(BODY_QUERY_SYNAPSE_COUNT).
      limit(m_numberLimit).
      ret(BODY_QUERY_RETURN);

  return queryNeuron(query.getQueryString());
}

ZJsonArray NeuPrintReader::queryNeuronCustom(const QString &condition)
{
  CypherQuery query = CypherQueryBuilder().
      init(condition).
      orderDesc(BODY_QUERY_SYNAPSE_COUNT).
      limit(m_numberLimit).
      ret(BODY_QUERY_RETURN);

  return queryNeuron(query.getQueryString());
}

ZJsonObject NeuPrintReader::customQuery(const QString &query)
{
  ZJsonObject resultObj;

  if (!query.isEmpty()) {
    QString url = m_server + "/api/custom/custom";
    m_bufferReader.post(url, query.toStdString().c_str());

    resultObj.decode(m_bufferReader.getBuffer().toStdString(), false);
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
  if (isReady()) {
    QString url = m_server + "/api/npexplorer/findneurons";

    ZJsonObject dataObj;
    dataObj.setEntry("dataset", m_currentDataset.toStdString());
    ZJsonArray inputRoiJson;
    for (const QString &inputRoi : inputRoiList) {
      inputRoiJson.append(inputRoi.toStdString());
    }
    dataObj.setEntry("input_ROIs", inputRoiJson);

    ZJsonArray outputRoiJson;
    for (const QString &outputRoi : outputRoiList) {
      outputRoiJson.append(outputRoi.toStdString());
    }
    dataObj.setEntry("output_ROIs", outputRoiJson);

    dataObj.setEntry("pre_threshold", 2);

    std::string queryStr = dataObj.dumpString(0);
    m_bufferReader.post(url, queryStr.c_str());

    KINFO << queryStr;

    return extract_body_list(m_bufferReader.getBuffer());
  }

  return QList<uint64_t>();
}


