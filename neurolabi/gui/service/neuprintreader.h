#ifndef NEUPRINTREADER_H
#define NEUPRINTREADER_H

#include <QString>
#include <QList>
#include <znetbufferreader.h>

class NeuPrintReader
{
public:
  NeuPrintReader(const QString server);

  void authorize(const QString &token);

  void readDatasets();
  QList<uint64_t> queryNeuron(const QString &inputRoi, const QString &outputRoi);

private:
  QString m_server;
  ZNetBufferReader m_bufferReader;
};

#endif // NEUPRINTREADER_H
