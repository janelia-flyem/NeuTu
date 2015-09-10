#ifndef ZFLYEMBODYANNOTATIONDIALOG_H
#define ZFLYEMBODYANNOTATIONDIALOG_H

#include <QDialog>
#include "tz_stdint.h"

class ZFlyEmBodyAnnotation;

namespace Ui {
class ZFlyEmBodyAnnotationDialog;
}

class ZFlyEmBodyAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBodyAnnotationDialog(QWidget *parent = 0);
  ~ZFlyEmBodyAnnotationDialog();

  ZFlyEmBodyAnnotation getBodyAnnotation() const;

  void loadBodyAnnotation(const ZFlyEmBodyAnnotation &annotation);

  void setBodyId(uint64_t bodyId);
  void setPrevUser(const std::string &name);

  uint64_t getBodyId() const;
  QString getComment() const;
  QString getName() const;
  QString getType() const;
  QString getStatus() const;

  void setComment(const std::string &comment);
  void setStatus(const std::string &status);
  void setName(const std::string &name);
  void setType(const std::string &type);

  void hideFinalizedStatus();
  void showFinalizedStatus();
  void freezeFinalizedStatus();
  void freezeUnknownStatus(const std::string &status);

public slots:
  void setNameEdit(const QString &name);

private:
  void connectSignalSlot();

private:
  Ui::ZFlyEmBodyAnnotationDialog *ui;
  uint64_t m_bodyId;

  static const std::string m_finalizedText;
};

#endif // ZFLYEMBODYANNOTATIONDIALOG_H
