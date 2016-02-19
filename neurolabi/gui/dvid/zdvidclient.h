#ifndef ZDVIDCLIENT_H
#define ZDVIDCLIENT_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>
#include <QVariant>
#include <QNetworkReply>
#include <QVector>
#include <QQueue>

#include "zobject3dscan.h"
#include "zswctree.h"
#include "zprogressable.h"
#include "zdvidrequest.h"
#include "zstack.hxx"
#include "dvid/zdvidtarget.h"

class ZDvidBuffer;

/*!
 * \brief The class if DVID client
 */
class ZDvidClient : public QObject, ZProgressable
{
  Q_OBJECT

public:
  ZDvidClient(QObject *parent = NULL);

  void setServer(const QString &server);
  void setUuid(const QString &uuid);
  void setPort(int port);
  void setServer(const QString &server, int port);

  void setDefaultServer();

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  inline void setDvidTarget(const ZDvidTarget &dvidTarget) {
    m_dvidTarget = dvidTarget;
  }

  /*
  inline const QString &getServer() {
    return m_serverAddress;
  }

  inline const QString& getUuid() {
    return m_uuid;
  }
  */

  inline const QString& getTmpDirectory() {
    return m_tmpDirectory;
  }



  /*!
   * \brief Send a request to DVID
   */
  bool postRequest(ZDvidRequest::EDvidRequest request, const QVariant &parameter);

  inline const ZObject3dScan& getObject() const { return m_obj; }
  inline const ZSwcTree& getSwcTree() const { return m_swcTree; }
  inline const ZStack& getImage() const { return m_image; }
  inline const QString& getInfo() const { return m_dataInfo; }
  inline const QByteArray& getKeyValue() const { return m_keyValue; }
  inline const QByteArray& getKeys() const { return m_keys; }

  inline ZDvidBuffer* getDvidBuffer() const { return m_dvidBuffer; }

  void appendRequest(ZDvidRequest request);

  //Make ready for making new requests
  void reset();

signals:
  void objectRetrieved();
  void swcRetrieved();
  void imageRetrieved();
  void infoRetrieved();
  void keyValueRetrieved();
  void keysRetrieved();
  void noRequestLeft();
  void requestFailed();
  void requestCanceled();
  void requestFinished();

public slots:
  void postNextRequest();

private slots:
  bool writeObject();
  void finishRequest(QNetworkReply::NetworkError error = QNetworkReply::NoError);
  void readObject();
  void readSwc();
  void readImage();
  void readInfo();
  void readKeyValue();
  void readKeys();
  void cancelRequest();

private:
  void createConnection();
  inline bool isCanceling() { return m_isCanceling; }

private:
  //QString m_serverAddress; //Server address
  //QString m_uuid; //uuid of the dataset
  //QString m_dataPath;
  ZDvidTarget m_dvidTarget;
  QNetworkAccessManager *m_networkManager;
  QNetworkReply *m_networkReply;
  QString m_targetDirectory;
  QString m_tmpDirectory;
  QFile *m_file;

  ZObject3dScan m_obj;
  ZSwcTree m_swcTree;
  ZStack m_image;
  QString m_dataInfo;
  QByteArray m_keyValue;
  QByteArray m_keys;
  QByteArray m_objectBuffer;
  QByteArray m_swcBuffer;
  QByteArray m_imageBuffer;
  QByteArray m_infoBuffer;
  QByteArray m_keyValueBuffer;
  QByteArray m_keysBuffer;

  QIODevice *m_uploadStream;

  ZDvidBuffer *m_dvidBuffer;
  QQueue<ZDvidRequest> m_requestQueue;
  ZDvidRequest m_currentRequest;


  bool m_isCanceling;
  //int m_requestIndex;
};

#endif // ZDVIDCLIENT_H
