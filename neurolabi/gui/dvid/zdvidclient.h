#ifndef ZDVIDCLIENT_H
#define ZDVIDCLIENT_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>
#include <QVariant>

#include "zobject3dscan.h"
#include "zprogressable.h"

/*!
 * \brief The class if DVID client
 */
class ZDvidClient : public QObject, ZProgressable
{
  Q_OBJECT

public:
  ZDvidClient(const QString &server, QObject *parent = NULL);

  enum EDvidRequest {
    DVID_GET_OBJECT, DVID_SAVE_OBJECT
  };

  inline void setServer(const QString &server) {
    m_serverAddress = server;
  }
  inline const QString &getServer() {
    return m_serverAddress;
  }

  /*!
   * \brief Send a request to DVID
   */
  bool postRequest(EDvidRequest request, const QVariant &parameter);

  inline const ZObject3dScan& getObject() const { return m_obj; }

signals:
  void objectRetrieved();

private slots:
  bool writeObject();
  void finishRequest();
  void readObject();

private:
  QString m_serverAddress; //Server address
  QNetworkAccessManager *m_networkManager;
  QNetworkReply *m_networkReply;
  QString m_targetDirectory;
  QFile *m_file;

  ZObject3dScan m_obj;
  QByteArray m_buffer;
};

#endif // ZDVIDCLIENT_H
