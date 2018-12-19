#ifndef NEUPRINTREADER_H
#define NEUPRINTREADER_H

#include <QString>
#include <QList>

#include "znetbufferreader.h"
#include "zjsonobject.h"

class ZJsonArray;

class NeuPrintReader
{
public:
  NeuPrintReader(const QString &server);

  void setServer(const QString &server);

  void authorize(const QString &token);
  void authorizeFromFile(const QString &filePath);

  void readDatasets();
  QList<uint64_t> queryNeuron(
      const QList<QString> &inputRoiList, const QList<QString> &outputRoiList);
  ZJsonArray findSimilarNeuron(const uint64_t bodyId);
  ZJsonArray queryNeuronByName(const QString &name);
  ZJsonArray queryAllNamedNeuron();
  ZJsonArray queryNeuronByStatus(const QString &status);

  ZJsonObject customQuery(const QString &query);
  ZJsonObject customQuery(const ZJsonObject &json);

  bool isReady();
  bool isConnected();
  bool isAuthorized() const;

  void updateCurrentDataset(const QString &uuid);

  QList<QString> getRoiList();

private:
  QString getNeuronLabel(char quote = '\0') const;

private:
  QString m_server;
  ZNetBufferReader m_bufferReader;
  ZJsonObject m_dataset;
  QString m_currentDataset;
  int m_numberLimit = 0;
};

#endif // NEUPRINTREADER_H
