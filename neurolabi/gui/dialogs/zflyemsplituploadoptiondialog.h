#ifndef ZFLYEMSPLITUPLOADOPTIONDIALOG_H
#define ZFLYEMSPLITUPLOADOPTIONDIALOG_H

#include <QDialog>

#include "dvid/zdvidreader.h"

namespace Ui {
class ZFlyEmSplitUploadOptionDialog;
}

class QStatusBar;
class ZFlyEmBodyAnnotation;

class ZFlyEmSplitUploadOptionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSplitUploadOptionDialog(QWidget *parent = 0);
  ~ZFlyEmSplitUploadOptionDialog();


  bool passingAnnotation() const;
  void setPassingAnnotation(bool on);
  bool newComment() const;
  void setNewComment(bool on);
  QString getComment() const;
  QString getStatus() const;

  void setComment(const QString &comment);

  void setDvidTarget(const ZDvidTarget &target);

//  ZFlyEmBodyAnnotation getAnnotation(uint64_t bodyId) const;
  void processAnnotation(ZJsonObject &annotation, bool usingDescription);

protected:
  bool event(QEvent *);

private:
  Ui::ZFlyEmSplitUploadOptionDialog *ui;
  ZDvidReader m_dvidReader;
//  QStatusBar *m_statusBar;
};

#endif // ZFLYEMSPLITUPLOADOPTIONDIALOG_H
