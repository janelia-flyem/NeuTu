#include "neuprintreader.h"

#include "zqslog.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

NeuPrintReader::NeuPrintReader(const QString &server) : m_server(server)
{
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

void NeuPrintReader::authorizeFromFile(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    ZJsonObject obj;
    obj.load(filePath.toStdString());
    std::string token = ZJsonParser::stringValue(obj["token"]);
    if (!token.empty()) {
      authorize(token.c_str());
    } else {
      LWARN() << "No NeuPrint token found.";
    }
  }
}

void NeuPrintReader::readDatasets()
{
  m_bufferReader.read(m_server + "/api/dbmeta/datasets", true);
  m_dataset.decodeString(m_bufferReader.getBuffer().toStdString().c_str());
  if (m_dataset.isEmpty()) {
    LWARN() << "No datasets retreived from NeuPrint.";
  }
}

bool NeuPrintReader::isAuthorized() const
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

bool NeuPrintReader::isReady()
{
  if (isConnected()) {
    return m_dataset.hasKey(m_currentDataset.toStdString());
  }

  return false;
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

void NeuPrintReader::updateCurrentDataset(const QString &uuid)
{
  m_currentDataset.clear();
  if (isConnected()) {
    const char *key = nullptr;
    json_t *value = nullptr;
    ZJsonObject_foreach(m_dataset, key, value) {
      ZJsonObject dataObj(value, ZJsonValue::SET_INCREASE_REF_COUNT);
      if (dataObj.hasKey("uuid")) {
        QString fullUuid(ZJsonParser::stringValue(dataObj["uuid"]).c_str());
        if (fullUuid.startsWith(uuid)){
          m_currentDataset = key;
        }
      }
    }
  }
}

namespace {
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
QList<uint64_t> NeuPrintReader::findSimilarNeuron(const uint64_t bodyId)
{
  QString url = m_server + "/api/custom/custom";
  ZJsonObject dataObj;
  dataObj.setEntry("dataset", "hemibrain");
  dataObj.setEntry("bodyId", bodyId);
  dataObj.setEntry("cypher", "MATCH (m:Meta{dataset:'hemibrain'}) "
                             "WITH m.superLevelRois AS rois "
                             "MATCH (n:`hemibrain-Neuron`{bodyId:"
                             + std::to_string(bodyId) +
                             "}) "
                             "WITH n.clusterName AS cn, rois "
                             "MATCH (n:`hemibrain-Neuron`{clusterName:cn}) "
                             "RETURN n.bodyId");
  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_list(m_bufferReader.getBuffer());
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

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  return extract_body_list(m_bufferReader.getBuffer());
}


