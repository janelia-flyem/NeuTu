#include "neuprintreader.h"

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

NeuPrintReader::NeuPrintReader(const QString server) : m_server(server)
{

}

void NeuPrintReader::authorize(const QString &token)
{
  m_bufferReader.setHeader("Authorization", QString("Bearer ") + token);
}

void NeuPrintReader::readDatasets()
{
  m_bufferReader.read(m_server + "/api/dbmeta/datasets", true);
  ZJsonObject dataObj;
  dataObj.decodeString(m_bufferReader.getBuffer().toStdString().c_str());
  dataObj.print();
}

QList<uint64_t> NeuPrintReader::queryNeuron(const QString &inputRoi, const QString &outputRoi)
{
  QString url = m_server + "/api/npexplorer/findneurons";

  ZJsonObject dataObj;
  dataObj.setEntry("dataset", "hemibrain");
  ZJsonArray inputRoiJson;
  inputRoiJson.append(inputRoi.toStdString());
  dataObj.setEntry("input_ROIs", inputRoiJson);

  ZJsonArray outputRoiJson;
  outputRoiJson.append(outputRoi.toStdString());
  dataObj.setEntry("output_ROIs", outputRoiJson);

  dataObj.setEntry("pre_threshold", 2);

  m_bufferReader.post(url, dataObj.dumpString(0).c_str());

  ZJsonObject resultObj;
  resultObj.decodeString(m_bufferReader.getBuffer().toStdString().c_str());

  resultObj.print();

  QList<uint64_t> bodyList;
  if (resultObj.hasKey("data")) {
    ZJsonArray data(resultObj.value("data"));
    for (size_t i = 0; i < data.size(); ++i) {
        uint64_t bodyId = ZJsonParser::integerValue(data.at(i), 0);
        bodyList.append(bodyId);
    }
  }

  return bodyList;
}
