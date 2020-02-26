#ifndef NEUPRINTREADER_H
#define NEUPRINTREADER_H

#include <QString>
#include <QList>

#include "common/neutudefs.h"
#include "qt/network/znetbufferreader.h"
#include "zjsonobject.h"

class ZJsonArray;

class NeuPrintReader
{
public:
  NeuPrintReader(const QString &server);

  QString getServer() const;
  void setServer(const QString &server);

  QString getToken() const;

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

  void updateCurrentDatasetFromUuid(const QString &uuid);
  bool hasDataset(const QString &uuid);
  QString getDataset(const QString &uuid);
  void setCurrentDataset(const QString &dataset);
  ZJsonObject getDatasetJson() const;
  QStringList getDatasetList() const;

  QList<QString> getRoiList();

  QString getNeuronLabel(char quote = '\0') const;

  neutu::EServerStatus getStatus() const;

private:
  QString getUuidKey(const QString &uuid);
  QString getCustomUrl() const;
  ZJsonObject getQueryJsonObject(const QString &query);
  ZJsonArray queryNeuron(const QString &query);

private:
  QString m_server;
  QString m_token;
  ZNetBufferReader m_bufferReader;
  ZJsonObject m_dataset;
  QString m_currentDataset;
  neutu::EServerStatus m_status = neutu::EServerStatus::OFFLINE;
  int m_numberLimit = 0;
};

#endif // NEUPRINTREADER_H
