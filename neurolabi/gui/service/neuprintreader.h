#ifndef NEUPRINTREADER_H
#define NEUPRINTREADER_H

#include <QString>
#include <QList>

#include "znetbufferreader.h"
#include "zjsonobject.h"

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
  QList<uint64_t> findSimilarNeuron(const uint64_t bodyId);

  bool isReady();
  bool isConnected();
  bool isAuthorized() const;

  void updateCurrentDataset(const QString &uuid);

  QList<QString> getRoiList();

private:
  QString m_server;
  ZNetBufferReader m_bufferReader;
  ZJsonObject m_dataset;
  QString m_currentDataset;
};

#endif // NEUPRINTREADER_H
