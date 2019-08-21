#include "protocolassignmentclient.h"

#include <QEventLoop>
#include <QMessageBox>

#include <QUrl>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


/**
 * this class is designed to interface with Rob's assignment manager
 * (see https://github.com/janelia-flyem/assignment-manager); it's a
 * rest service, so this is basically a rest client; my intention is to
 * keep it stateless
 *
 * Qt really wants its network calls to be asynchronous, but the use case
 * for this class is to get tasks; the user *must* wait for the response
 * before doing anything!  also, it should be quite fast (small data and
 * no calculation) to return; as such, I've thrown a wait loop around
 * the calls to make them effectively synchronous, because doing it full-on
 * async would make the logic in callers far, far more convoluted
 *
 * this class is intended to be largely protocol-independent
 */

ProtocolAssignmentClient::ProtocolAssignmentClient(QObject *parent) : QObject(parent)
{

    // record username here?

    m_networkManager = new QNetworkAccessManager(this);

}

void ProtocolAssignmentClient::setServer(QString server) {
    m_server = server;    
    if (!m_server.endsWith("/")) {
        m_server.append("/");
    }
}

QMap<QString, int> ProtocolAssignmentClient::getProjectsForProtocol(AssigmentProtocols protocol) {

    // get all projects for the input protocol


    // url: http://flyem-assignment.int.janelia.org/projects?protocol=orphan_link
    /*
    {
      "rest": {
        "requester": "172.31.19.4",
        "url": "http://flyem-assignment.int.janelia.org/projects?protocol=orphan_link",
        "endpoint": "get_project_info",
        "error": false,
        "elapsed_time": "0:00:00.006644",
        "row_count": 3,
        "pid": 18,
        "sql_statement": "SELECT * FROM project_vw WHERE protocol=('orphan_link')"
      },
      "data": [
        {
          "id": 1,
          "name": "Project 1",
          "protocol": "orphan_link",
          "priority": 3,
          "disposition": null,
          "note": null,
          "create_date": "Tue, 20 Aug 2019 11:21:00"
        },
        ...
        ]
        }
    */

    // construct url
    // inlined for testing; I expect to break this out later:
    QString url = m_server;
    if (protocol == ORPHAN_LINK) {
        url += "projects?protocol=orphan_link";
    } else {

        // not supported yet

    }


    // make call
    QJsonObject result = get(url);


    // check for errors; parse
    QMap<QString, int> projects;
    QJsonObject restData = result["rest"].toObject();
    if (restData["error"].toBool()) {
        showError("error getting projects", "Error in retrieving projects: " + restData["message"].toString());
    } else {
        QJsonArray data = result["data"].toArray();
        for (QJsonValue val: data) {
            QJsonObject valObj = val.toObject();
            projects[valObj["name"].toString()] = valObj["id"].toInt();
        }
    }
    return projects;
}

QJsonObject ProtocolAssignmentClient::get(QString url) {

    // see comments at top as to why this has been implemented synchronously

    // url is correct here:
    qDebug() << "ProtocolAssignmentClient::get url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);
    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(requestUrl));

    // Qt synchronous network call requires mini-event loop, ugh; from examples online:
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "get error (number below?)";
        qDebug() << reply->error();
        QJsonObject errorObject;
        errorObject["error"] = true;
        errorObject["url"] = url;
        errorObject["message"] = "QNetworkReply error code: " + QString::number(reply->error());
        return errorObject;
    } else {
        qDebug() << "get success (reply below?):";
        QString stringReply = (QString) reply->readAll();
        qDebug() << stringReply;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(stringReply.toUtf8());
        return jsonResponse.object();
    }
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void ProtocolAssignmentClient::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}
