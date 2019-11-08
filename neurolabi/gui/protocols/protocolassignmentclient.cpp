#include "protocolassignmentclient.h"

#include <QEventLoop>
#include <QMessageBox>

#include <QUrl>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "protocolassignmenturl.h"


/*
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

/*
 * retrieves projects for a protocol
 *
 * endpoint: get /project/{protocol}
 * input: protocol (from enum)
 * output: map of project name: project id
 */
QMap<QString, int> ProtocolAssignmentClient::getProjectsForProtocol(AssigmentProtocols protocol) {

    QMap<QString, int> projects;



    // this call needs to restrict the returned list to those the user is allowed to generate;
    //  /projects/eligible is the endpoint for that, returning list of projects (name or id?)

    // currently that endpoint needs an auth token (that's where it gets the username from),
    //  so we can't do that yet


    // construct url
    QString url;
    if (protocol == ORPHAN_LINK) {
        url = ProtocolAssignmentUrl::GetProjectsForProtocol(m_server, "orphan_link");
    } else {
        showError("Unknown protocol", "Unknown protocol!");
        return projects;
    }

    // make call
    QJsonObject result = get(url);
    QJsonObject restData = result["rest"].toObject();

    if (restData["error"].toBool()) {
        showError("Error getting projects", "Error in retrieving projects: " + restData["message"].toString());
    } else {
        QJsonArray data = result["data"].toArray();
        for (QJsonValue val: data) {
            QJsonObject valObj = val.toObject();
            projects[valObj["name"].toString()] = valObj["id"].toInt();
        }
    }
    return projects;
}

/*
 * generates an assignment in a project (does not start it)
 *
 * endpoint: post /assignment/{project name}
 * input: project name
 * output: assigment ID
 */
int ProtocolAssignmentClient::generateAssignment(QString projectName) {

    // needs authentication, not working yet

    return 123;



    // in progress:

    QString url = ProtocolAssignmentUrl::GenerateAssignment(m_server, projectName);

    // auth goes into the data?
    QJsonObject data;
    QJsonObject result = post(url, data);


    // error handling

    // parse result and return



}

bool ProtocolAssignmentClient::startAssignment(int assignmentID) {

    // needs auth, not working

    return true;


    // in progress:

    QString url = ProtocolAssignmentUrl::StartAssignment(m_server, assignmentID);

    QJsonObject data;
    QJsonObject result = post(url, data);


    // errors, parsing, etc...


}

QJsonObject ProtocolAssignmentClient::get(QString url) {
    qDebug() << "ProtocolAssignmentClient::get: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);
    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(requestUrl));
    return call(reply);
}

QJsonObject ProtocolAssignmentClient::post(QString url, QJsonObject jsonData) {
    // see general comments in get(), above

    qDebug() << "ProtocolAssignmentClient::put: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);

    QJsonDocument jsonDataDoc(jsonData);
    QByteArray byteData(jsonDataDoc.toJson());

    QNetworkReply *reply = m_networkManager->post(QNetworkRequest(requestUrl), byteData);
    return call(reply);
}

QJsonObject ProtocolAssignmentClient::call(QNetworkReply * reply) {

    // see comments at top as to why this has been implemented synchronously
    // Qt synchronous network call requires mini-event loop, ugh; from examples online:
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        // this mimics the form of the return from the assignment manager
        QJsonObject errorObject;
        errorObject["error"] = true;
        errorObject["url"] = reply->request().url().toDisplayString();
        errorObject["message"] = "QNetworkReply error code: " + QString::number(reply->error());
        return errorObject;
    } else {
        QString stringReply = (QString) reply->readAll();
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
