#ifndef NEUPRINTREADER_H
#define NEUPRINTREADER_H

#include <QString>
#include <QList>

#include "qt/network/znetbufferreader.h"
#include "zjsonobject.h"

class ZJsonArray;

class NeuPrintReader
{
public:
  NeuPrintReader(const QString &server);

  QString getServer() const;
  void setServer(const QString &server);

  void authorize(const QString &token);
  void authorizeFromFile(const QString &filePath);
  void authorizeFromJson(const QString &auth);

  void readDatasets();
  QList<uint64_t> queryNeuron(
      const QList<QString> &inputRoiList, const QList<QString> &outputRoiList);
  ZJsonArray findSimilarNeuron(const uint64_t bodyId);
  ZJsonArray queryNeuronByName(const QString &name);
  ZJsonArray queryNeuronByType(const QString &type);
  ZJsonArray queryAllNamedNeuron();
  ZJsonArray queryNeuronByStatus(const QString &status);
  ZJsonArray queryNeuronByStatus(const QList<QString> &statusList);
  ZJsonArray queryTopNeuron(int n);
  ZJsonArray queryNeuronCustom(const QString &condition);

  ZJsonObject customQuery(const QString &query);
  ZJsonObject customQuery(const ZJsonObject &json);

  bool isReady();
  bool isConnected();
  bool hasAuthCode() const;
  bool connect();

  void updateCurrentDataset(const QString &uuid);
  bool hasDataset(const QString &uuid);
  ZJsonObject getDatasetJson() const;
  QStringList getDatasetList() const;

  QList<QString> getRoiList();

private:
  QString getNeuronLabel(char quote = '\0') const;
  QString getUuidKey(const QString &uuid);
  QString getCustomUrl() const;
  ZJsonObject getQueryJsonObject(const QString &query);
  ZJsonArray queryNeuron(const QString &query);

private:
  QString m_server;
  ZNetBufferReader m_bufferReader;
  ZJsonObject m_dataset;
  QString m_currentDataset;
  int m_numberLimit = 0;
};

#endif // NEUPRINTREADER_H
