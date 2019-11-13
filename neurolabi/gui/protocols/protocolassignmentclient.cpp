#include "protocolassignmentclient.h"

#include <QEventLoop>
#include <QMessageBox>

#include <QUrl>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "neutube.h"

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
 *
 * for now, the methods in this class try to handle errors internally,
 * often showing dialogs, then returning empty values if there were errors;
 * this is not ideal, but it is quick and convenient to implement
 */

ProtocolAssignmentClient::ProtocolAssignmentClient(QObject *parent) : QObject(parent)
{

    m_networkManager = new QNetworkAccessManager(this);

}

void ProtocolAssignmentClient::setServer(QString server) {
    m_server = server;    
    if (m_server.endsWith("/")) {
        m_server.chop(1);
    }
}

void ProtocolAssignmentClient::setToken(QString token) {
    m_token = token;
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


    /*


    // not updated for new error handling!



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
    */

    return projects;
}

/*
 * retrieve eligible projects for a user
 *
 * endpoint: get /projects/eligible
 * input: none
 * output: list of project names
 */
QStringList ProtocolAssignmentClient::getEligibleProjects() {
    QStringList projects;

    QString url = ProtocolAssignmentUrl::GetEligibleProjects(m_server);
    QNetworkReply * reply = get(url);

    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving eligible projects: " + error);
        return projects;
    } else {
        // you know, I've never seen this succeed...I assume it's a json array...

        QJsonArray data = getReplyDataArray(reply);

        // test data
        projects << "project 1" << "project 2" << "project 3";

        return projects;

    }
}

/*
 * retrieve started assignments for a user
 *
 * endpoint: /assignments_started?user=username
 * input: none
 * output: array of assignment objects
 */
QJsonArray ProtocolAssignmentClient::getStartedAssignments() {
    QJsonArray array;

    // need to change Janelia username into assignment mgr username
    QString username = getLocalUsername(QString::fromStdString(neutu::GetCurrentUserName()));
    QString url = ProtocolAssignmentUrl::GetStartedAssigments(m_server, username);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving started assignments: " + error);
        return array;
    } else {
        return getReplyDataArray(reply);
    }
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
    QNetworkReply * reply = post(url, data);


    // error handling

    // parse result and return



}

bool ProtocolAssignmentClient::startAssignment(int assignmentID) {

    // needs auth, not working

    return true;


    // in progress:

    QString url = ProtocolAssignmentUrl::StartAssignment(m_server, assignmentID);

    QJsonObject data;
    QNetworkReply* reply = post(url, data);


    // errors, parsing, etc...


}

/*
 * endpoint: /users?janelia_id=username
 * input: janelia username
 * output: username in the assignment manager (Google login username);
 *      returns empty string if it couldn't be determined
 */
QString ProtocolAssignmentClient::getLocalUsername(QString janeliaUsername) {
    QString url = ProtocolAssignmentUrl::GetJaneliaUser(m_server, janeliaUsername);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error determining assignment manager username for Janelia username: " + error);
        return "";
    } else {
        // data should be an array with exactly one object
        QJsonArray data = getReplyDataArray(reply);
        if (data.size() == 0) {
            // I think this never happens; it'll show as an error (404) instead
            showError("Error!", "Error determining assignment manager username for Janelia username!");
            return "";
        } else if (data.size() > 1) {
            // I really hope this is also impossible
            showError("Error!", "Error determining assignment manager username for Janelia username! More than one value found@");
            return "";
        } else {
            QJsonObject userData = data[0].toObject();
            return userData["name"].toString();
        }
    }
}

QNetworkReply * ProtocolAssignmentClient::get(QString url) {
    qDebug() << "ProtocolAssignmentClient::get: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);    
    QNetworkRequest request = QNetworkRequest(requestUrl);
    if (!m_token.isEmpty()) {
        QString authString = "Bearer " + m_token;
        request.setRawHeader(QByteArray("Authorization"), authString.toUtf8());
    }
    QNetworkReply *reply = m_networkManager->get(request);
    return call(reply);
}

QNetworkReply * ProtocolAssignmentClient::post(QString url, QJsonObject jsonData) {
    qDebug() << "ProtocolAssignmentClient::put: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);

    QJsonDocument jsonDataDoc(jsonData);
    QByteArray byteData(jsonDataDoc.toJson());

    QNetworkRequest request = QNetworkRequest(requestUrl);
    if (!m_token.isEmpty()) {
        QString authString = "Bearer " + m_token;
        request.setRawHeader(QByteArray("Authorization"), authString.toUtf8());
    }
    QNetworkReply *reply = m_networkManager->post(request, byteData);
    return call(reply);
}

QNetworkReply * ProtocolAssignmentClient::call(QNetworkReply * reply) {
    // see comments at top as to why this has been implemented synchronously
    // Qt synchronous network call requires mini-event loop, ugh; from examples online:
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    qDebug() << "ProtocolAssignmentClient::call: http status = " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (reply->error() != QNetworkReply::NoError) {
        // this mimics the form of the return from the assignment manager
        qDebug() << "error reported: " << reply->errorString();
    }

    return reply;
}

bool ProtocolAssignmentClient::hadError(QNetworkReply *reply) {
    return reply->error() != QNetworkReply::NoError;
}

/*
 * return error string; at worst, it'll be the string Qt returns; if
 * the call "succeeds" in returning json, it'll return the error string
 * from there (the error that comes from the server in ["rest"]["error"])
 */
QString ProtocolAssignmentClient::getErrorString(QNetworkReply *reply) {
    // the default is Qt's error string:
    QString errorString;
    if (reply->error() == QNetworkReply::NoError) {
        return errorString;
    }
    errorString = reply->errorString();

    // now check the reply; if we can parse it, we'll return that error instead
    QString stringReply = (QString) reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(stringReply.toUtf8());
    if (!doc.isNull()) {
        QJsonObject obj = doc.object();
        QJsonObject restData = obj["rest"].toObject();
        if (restData["error"].isString()) {
            errorString = restData["error"].toString();
        }
    }
    return errorString;
}

int ProtocolAssignmentClient::getReplyStatus(QNetworkReply *reply) {
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

/*
 * this method gives you all the json from the reply, including both the
 * "rest" and "data" parts
 */
QJsonObject ProtocolAssignmentClient::getReplyJSON(QNetworkReply *reply) {
    QString stringReply = (QString) reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(stringReply.toUtf8());
    return jsonResponse.object();
}

/*
 * return only the "data" part of the reply if you know it's a JSON object
 */
QJsonObject ProtocolAssignmentClient::getReplyDataObject(QNetworkReply *reply) {
    return getReplyJSON(reply)["data"].toObject();
}

/*
 * return only the "data" part of the reply if you know it's a JSON array
 */
QJsonArray ProtocolAssignmentClient::getReplyDataArray(QNetworkReply *reply) {
    return getReplyJSON(reply)["data"].toArray();
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
