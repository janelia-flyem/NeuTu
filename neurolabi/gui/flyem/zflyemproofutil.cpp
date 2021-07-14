#include "zflyemproofutil.h"

#include <QString>

#include "zdialogfactory.h"
#include "zjsonobjectparser.h"
#include "zflyemproofdoc.h"
#include "zflyembodyannotation.h"
#include "zflyembodyannotationprotocol.h"

ZFlyEmProofUtil::ZFlyEmProofUtil()
{
}

namespace {
bool confirmStatus(
    uint64_t bodyId,
    const std::string &status, const std::string &oldStatus,
    ZFlyEmProofDoc *doc, QWidget *parentWidget)
{
  bool okToContinue = true;
  ZFlyEmBodyAnnotationProtocol protocal = doc->getBodyStatusProtocol();
  if (protocal.preservingId(oldStatus)) {
    QString warnMsg;

    if (protocal.preservingId(status)) {
      if (protocal.getStatusRank(status) > protocal.getStatusRank(oldStatus)) { //demoting
        warnMsg =
            QString("You are demoting the status of "
                    "the preserved body %1 (<font color=\"#0000FF\">%2</font>"
                    " => <font color=\"#0000FF\">%3</font>).").
            arg(bodyId).
            arg(QString::fromStdString(oldStatus), QString::fromStdString(status));
      }
    } else {
      warnMsg =
          QString("You are changing an id-preserving status "
                  "to  a non-preserving one "
                  "(<font color=\"#0000FF\">%1</font> => "
                  "<font color=\"#0000FF\">%2</font>).").
          arg(oldStatus.c_str(), status.c_str());
    }

    if (!warnMsg.isEmpty()) {
      okToContinue = ZDialogFactory::Ask(
            "Risk Detected",
            "<font color=\"#FF0000\">WARNING</font>: " + warnMsg +
            "<br><br>Do you want to continue?",
            parentWidget);
    }
  }

  return okToContinue;
}

}

namespace {

template <typename T>
bool annotate_body(
      uint64_t bodyId, const T &annotation,
      const T &oldAnnotation,
      ZFlyEmProofDoc *doc, QWidget *parentWidget)
{
  ZFlyEmBodyAnnotationProtocol protocal = doc->getBodyStatusProtocol();
  std::string status = ZFlyEmBodyAnnotation::GetStatus(annotation);
  std::string oldStatus = ZFlyEmBodyAnnotation::GetStatus(oldAnnotation);
  bool okToContinue = confirmStatus(
        bodyId, status, oldStatus, doc, parentWidget);

  if (okToContinue) {
    doc->annotateBody(bodyId, annotation);
  }

  return okToContinue;
}

}

bool ZFlyEmProofUtil::AnnotateBody(
    uint64_t bodyId,
    const ZFlyEmBodyAnnotation &annotation,
    const ZFlyEmBodyAnnotation &oldAnnotation,
    ZFlyEmProofDoc *doc,
    QWidget *parentWidget)
{
  return annotate_body(bodyId, annotation, oldAnnotation, doc, parentWidget);
  /*
  ZFlyEmBodyAnnotationProtocol protocal = doc->getBodyStatusProtocol();
  bool okToContinue = confirmStatus(
        bodyId, annotation.getStatus(), oldAnnotation.getStatus(),
        doc, parentWidget);

  if (okToContinue) {
    doc->annotateBody(bodyId, annotation);
  }

  return okToContinue;
  */
}

bool ZFlyEmProofUtil::AnnotateBody(
      uint64_t bodyId, const ZJsonObject &annotation,
      const ZJsonObject &oldAnnotation,
      ZFlyEmProofDoc *doc, QWidget *parentWidget)
{
  return annotate_body(bodyId, annotation, oldAnnotation, doc, parentWidget);
  /*
  ZFlyEmBodyAnnotationProtocol protocal = doc->getBodyStatusProtocol();
  std::string status = ZJsonObjectParser::GetValue(annotation, "status", "");
  std::string oldStatus = ZJsonObjectParser::GetValue(oldAnnotation, "status", "");
  bool okToContinue = confirmStatus(
        bodyId, status, oldStatus, doc, parentWidget);

  if (okToContinue) {
    doc->annotateBody(bodyId, annotation);
  }

  return okToContinue;
  */
}
