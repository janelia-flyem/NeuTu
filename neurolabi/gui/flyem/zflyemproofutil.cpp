#include "zflyemproofutil.h"

#include <QString>

#include "zdialogfactory.h"

#include "zflyemproofdoc.h"
#include "zflyembodyannotation.h"
#include "zflyembodyannotationprotocol.h"

ZFlyEmProofUtil::ZFlyEmProofUtil()
{

}

bool ZFlyEmProofUtil::AnnotateBody(
    uint64_t bodyId,
    const ZFlyEmBodyAnnotation &annotation,
    const ZFlyEmBodyAnnotation &oldAnnotation,
    ZFlyEmProofDoc *doc,
    QWidget *parentWidget)
{
  ZFlyEmBodyAnnotationProtocal protocal = doc->getBodyStatusProtocol();
  bool okToContinue = true;
  if (!oldAnnotation.isEmpty() &&
      protocal.preservingId(oldAnnotation.getStatus())) {
    QString warnMsg;

    if (protocal.preservingId(annotation.getStatus())) {
      if (protocal.getStatusRank(annotation.getStatus()) >
          protocal.getStatusRank(oldAnnotation.getStatus())) { //demoting
        warnMsg =
            QString("You are demoting the status of "
                    "the preserved body %1 (<font color=\"#0000FF\">%2</font>"
                    " => <font color=\"#0000FF\">%3</font>).").
            arg(bodyId).
            arg(oldAnnotation.getStatus().c_str()).
            arg(annotation.getStatus().c_str());
      }
    } else {
      warnMsg =
          QString("You are changing an id-preserving status "
                  "to  a non-preserving one "
                  "(<font color=\"#0000FF\">%1</font> => "
                  "<font color=\"#0000FF\">%2</font>).").
          arg(oldAnnotation.getStatus().c_str()).
          arg(annotation.getStatus().c_str());
    }

    if (!warnMsg.isEmpty()) {
      okToContinue = ZDialogFactory::Ask(
            "Risk Detected",
            "<font color=\"#FF0000\">WARNING</font>: " + warnMsg +
            "<br><br>Do you want to continue?",
            parentWidget);
    }
  }

  if (okToContinue) {
    doc->annotateBody(bodyId, annotation);
  }

  return okToContinue;
}
