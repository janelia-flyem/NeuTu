#include "flyembodyannotationgenericdlgbuilder.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

#include "dialogs/zgenericbodyannotationdialog.h"
#include "zflyembodyannotation.h"
#include "neutube.h"

FlyEmBodyAnnotationGenericDlgBuilder::FlyEmBodyAnnotationGenericDlgBuilder()
{

}

FlyEmBodyAnnotationGenericDlgBuilder&
FlyEmBodyAnnotationGenericDlgBuilder::getDialogFrom(
    std::function<ZGenericBodyAnnotationDialog*()> f, bool ready)
{
  m_dlgGetter = f;
  m_dlgReady = ready;

  return *this;
}

ZSegmentAnnotationBuilder&
FlyEmBodyAnnotationGenericDlgBuilder::fromOldAnnotation(const ZJsonObject &obj)
{
  m_annotation = ZJsonObject::MakeNull();
  if (m_dlgGetter) {
    ZGenericBodyAnnotationDialog *dlg = m_dlgGetter();
    if (dlg) {
      bool dlgReady = m_dlgReady;
      if (dlgReady) {
        m_annotation = dlg->toJsonObject();
      } else {
        dlg->loadJsonObject(obj);

        QString label = (m_bodyId > 0) ? QString("Body ID: <b>%1</b>").arg(m_bodyId) : "";

        auto info = getUserTimeInfo(obj);
        label += "<br><br>Last modified by: ";
        if (!info.contains("latest_time")) {
            if (info.size() == 0) {
                label += "(no info)";
            } else {
                label += "(hover for info)";
            }
        } else {
            label += info["latest_user"] + "<br> (hover for more info)";
        }
        QString tooltip;
        for (auto key: info.keys()) {
            if (!key.startsWith("latest")) {
                tooltip += key + ": " + info[key] + "<br>";
            }
        }
        // lazy cleanup of trailing "<br>":
        if (!tooltip.isEmpty()) {
            tooltip.truncate(tooltip.size() - 4);
        }

        if (!label.isEmpty()) {
          dlg->setLabel(label, tooltip);
        }
        if (dlg->exec()) {
          m_annotation = dlg->toJsonObject();
          dlgReady = true;
        }
      }
      if (dlgReady) {
        std::string user = neutu::GetCurrentUserName();
        ZFlyEmBodyAnnotation::UpdateUserFields(
              m_annotation, user, obj);
      }
    }
  }

  if (!m_annotation.shellOnly()) {
    m_annotation =
        ZSegmentAnnotationBuilder().copy(obj).join(m_annotation).withoutNull();

    // remove _user and _time fields from annotation; DVID will add as needed
    for (std::string key: m_annotation.getAllKey()) {
        if (key.length() > 5) {
            std::string end5 = key.substr(key.length() - 5);
            if (end5 == "_user" || end5 == "_time") {
                m_annotation.removeKey(key.c_str());
            }
        }
    }

  }

  return *this;
}

QMap<QString, QString> FlyEmBodyAnnotationGenericDlgBuilder::getUserTimeInfo(const ZJsonObject &obj) {
    QMap<QString, QString> info;
    // returns a map of all fields ending with _user or _time
    // in "latest_time", returns the most recent of the _time entries
    // in "latest_user", returns the _user corresponding to latest _time entry

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(obj.dumpString()).toUtf8());
    QJsonObject data = doc.object();

    QString latestTimeKey = "";
    QDateTime initialTime = QDateTime::fromString("2000-01-01", "yyyy-MM-dd");
    QDateTime latestTime = initialTime;
    for (QString key: data.keys()) {
        if (key.endsWith("_user")) {
            info[key] = data[key].toString();
        } else if (key.endsWith("_time")) {
            QString timeString = data[key].toString();
            info[key] = timeString;
            // time format: 2023-05-09T09:45:33-04:00 = Qt::ISODate
            QDateTime time = QDateTime::fromString(timeString, Qt::ISODate);
            if (time > latestTime) {
                latestTime = time;
                latestTimeKey = key;
            }
        }
    }
    if (latestTime != initialTime) {
        info["latest_time"] = latestTimeKey;
        QString userString = latestTimeKey.replace("_time", "_user");
        info["latest_user"] = info[userString];
    }

    return info;
}
