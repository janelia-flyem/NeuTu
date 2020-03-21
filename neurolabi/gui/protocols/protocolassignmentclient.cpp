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

#include "flyem/auth/flyemauthtokenhandler.h"


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

const QString ProtocolAssignmentClient::ASSIGNMENT_APPLICATION_NAME = "assignment-manager";

void ProtocolAssignmentClient::setServer(QString server) {
    m_server = server;    
    if (m_server.endsWith("/")) {
        m_server.chop(1);
    }
    checkForTokens();
}

bool ProtocolAssignmentClient::checkForTokens() {
    // check for master token
    FlyEmAuthTokenHandler handler;
    if (!handler.hasMasterToken()) {
        showError("No authentication token!",
            "NeuTu needs your Fly EM services authentication token! "
            "Open the authentication dialog via the yellow key icon "
            "on the toolbar and follow the instructions, "
            "then try this action again.");
        return false;
    }

    // if present, try to get application token
    QString token = handler.getApplicationToken(ASSIGNMENT_APPLICATION_NAME);
    if (token.isEmpty()) {
        showError("No application token!",
                  "Could not retrieve application token for " +
                  ASSIGNMENT_APPLICATION_NAME);
        return false;
    }
    m_token = token;
    return true;
}

/*
 * retrieve eligible projects for a user
 *
 * endpoint: get /projects/eligible
 * input: none
 * output: map of project name: protocol
 */
QMap<QString, QString> ProtocolAssignmentClient::getEligibleProjects() {
    QMap<QString, QString> projects;

    QString url = ProtocolAssignmentUrl::GetEligibleProjects(m_server);
    QNetworkReply * reply = get(url);

    if (hadError(reply)) {
        // if it's a 404 (with message "No eligible projects"), it's not really an error,
        //  just an indication of no results; return
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 404) {
            return projects;
        } else {
            showError("Error!", "Error retrieving eligible projects: " + getErrorString(reply));
            return projects;
        }
    } else {
        QJsonObject data = getReplyJsonObject(reply, "projects");
        for (QString key: data.keys()) {
            QString val = data.value(key).toString();
            projects[key] = val;
        }
        return projects;
    }
}

/*
 * retrieve one assignment by id
 *
 * endpoint: get /assignment/(assignment id)
 * input: assignment ID
 * output: assignment object
 */
ProtocolAssignment ProtocolAssignmentClient::getAssignment(int assignmentID) {
    QString url = ProtocolAssignmentUrl::GetAssignment(m_server, assignmentID);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving assignment: " + error);
        QJsonObject empty;
        return ProtocolAssignment(empty);
    } else {
        QJsonArray array = getReplyJsonArray(reply, "data");
        if (array.size() != 1) {
            showError("Error!", "Retrieved " + QString::number(array.size()) + " assignments instead of one!" );
            QJsonObject empty;
            return ProtocolAssignment(empty);
        }
        QJsonObject data = array[0].toObject();
        return ProtocolAssignment(data);
    }
}

/*
 * retrieve all assignments for a user
 *
 * endpoint: /assignments?user=username
 * input: none
 * output: list of assignment objects
 */
QList<ProtocolAssignment> ProtocolAssignmentClient::getAssignments() {
    QList<ProtocolAssignment> results;

    // need to change Janelia username into assignment mgr username
    QString username = getLocalUsername(QString::fromStdString(neutu::GetCurrentUserName()));
    QString url = ProtocolAssignmentUrl::GetAssigments(m_server, username);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving assignments: " + error);
        return results;
    } else {
        QJsonArray array = getReplyJsonArray(reply, "data");
        for (QJsonValue val: array) {
            results << ProtocolAssignment(val.toObject());
        }
        return results;
    }
}

/*
 * retrieve started assignments for a user
 *
 * endpoint: /assignments_started?user=username
 * input: none
 * output: list of assignment objects
 */
QList<ProtocolAssignment> ProtocolAssignmentClient::getStartedAssignments() {
    QList<ProtocolAssignment> results;

    // need to change Janelia username into assignment mgr username
    QString username = getLocalUsername(QString::fromStdString(neutu::GetCurrentUserName()));
    QString url = ProtocolAssignmentUrl::GetStartedAssigments(m_server, username);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving started assignments: " + error);
        return results;
    } else {
        QJsonArray array = getReplyJsonArray(reply, "data");
        for (QJsonValue val: array) {
            results << ProtocolAssignment(val.toObject());
        }
        return results;
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
    QString url = ProtocolAssignmentUrl::GenerateAssignment(m_server, projectName);
    int assignmentID = -1;

    QJsonObject data;
    QNetworkReply * reply = post(url, data);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving started assignments: " + error);
        QJsonObject empty;
        return assignmentID;
    } else {
        // parse reply; get ID, then get the full assignment object and return it

        // I'm of two minds whether I should bother returning the object or just the new ID;
        //  currently, I don't use either, except as a success indicator; the UI
        //  refreshes and gets the full assignment info itself later

        QJsonObject rest = getReplyJsonObject(reply, "rest");
        return rest["inserted_id"].toInt();
    }
}

/*
 * endpoint: post /assignment/{assignment_id}/start
 * input: assignment ID
 * output: success
 */
bool ProtocolAssignmentClient::startAssignment(int assignmentID) {
    QString url = ProtocolAssignmentUrl::StartAssignment(m_server, assignmentID);
    QJsonObject data;
    QNetworkReply* reply = post(url, data);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error completing assignment: " + error);
        return false;
    } else {
        return true;
    }
}

/*
 * endpoint: /assignment/{assignment_id}/complete
 * input: assignment
 * output: boolean success
 */
bool ProtocolAssignmentClient::completeAssignment(ProtocolAssignment assignment) {
    QString url = ProtocolAssignmentUrl::CompleteAssignment(m_server, assignment.id);
    QJsonObject empty;
    QNetworkReply * reply = post(url, empty);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error completing assignment: " + error);
        return false;
    } else {
        return true;
    }
}

/*
 * endpoint: /tasks?assignment_id=12345
 * input: assignment object
 * output: list of task objects
 */
QList<ProtocolAssignmentTask> ProtocolAssignmentClient::getAssignmentTasks(ProtocolAssignment assignment) {
    QList<ProtocolAssignmentTask> results;

    QString url = ProtocolAssignmentUrl::GetAssignmentTasks(m_server, assignment.id);
    QNetworkReply * reply = get(url);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error retrieving assignment tasks: " + error);
        return results;
    } else {
        QJsonArray array = getReplyJsonArray(reply, "data");
        for (QJsonValue val: array) {
            results << ProtocolAssignmentTask(val.toObject());
        }
        return results;
    }
}

/*
 * endpoint: /task/{task_id}/start
 * input: task
 * output: boolean success
 */
bool ProtocolAssignmentClient::startTask(ProtocolAssignmentTask task) {
    QString url = ProtocolAssignmentUrl::StartTask(m_server, task.id);
    QJsonObject empty;
    QNetworkReply * reply = post(url, empty);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error starting task: " + error);
        return false;
    } else {
        return true;
    }
}

/*
 * endpoint: /task/{task_id}/complete
 * input: task
 * output: boolean success
 */
bool ProtocolAssignmentClient::completeTask(ProtocolAssignmentTask task, bool skipped, QString note) {
    QString url = ProtocolAssignmentUrl::CompleteTask(m_server, task.id);
    QJsonObject data;
    if (skipped) {
        data["disposition"] = ProtocolAssignmentTask::DISPOSITION_SKIPPED;
    }
    if (!note.isEmpty()) {
        data["note"] = note;
    }
    QNetworkReply * reply = post(url, data);
    if (hadError(reply)) {
        QString error = getErrorString(reply);
        showError("Error!", "Error completing task: " + error);
        return false;
    } else {
        return true;
    }
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
        QJsonArray data = getReplyJsonArray(reply, "data");
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
    qDebug() << "ProtocolAssignmentClient::post: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);

    QJsonDocument jsonDataDoc(jsonData);
    QByteArray byteData(jsonDataDoc.toJson());

    QNetworkRequest request = QNetworkRequest(requestUrl);
    if (!m_token.isEmpty()) {
        QString authString = "Bearer " + m_token;
        request.setRawHeader(QByteArray("Authorization"), authString.toUtf8());
    }
    request.setRawHeader("Content-Type", "application/json");
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

    if (reply->error() != QNetworkReply::NoError) {
        // on error, log some things before returning
        qDebug() << "ProtocolAssignmentClient::call: http status = " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug() << "Qt error string: " << reply->errorString();
        qDebug() << "processed error string: " << getErrorString(reply);
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
 * return a JSON object from the reply from the given key
 */
QJsonObject ProtocolAssignmentClient::getReplyJsonObject(QNetworkReply *reply, QString key) {
    QJsonObject json = getReplyJSON(reply);
    if (json.contains(key) && json[key].isObject()) {
        return json[key].toObject();
    } else {
        return QJsonObject();
    }
}

/*
 * return only the "data" part of the reply if you know it's a JSON array
 */
QJsonArray ProtocolAssignmentClient::getReplyJsonArray(QNetworkReply *reply, QString key) {
    QJsonObject json = getReplyJSON(reply);
    if (json.contains(key) && json[key].isArray()) {
        return json[key].toArray();
    } else {
        return QJsonArray();
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
