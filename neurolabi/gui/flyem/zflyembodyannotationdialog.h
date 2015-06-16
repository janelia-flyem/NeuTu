#ifndef ZFLYEMBODYANNOTATIONDIALOG_H
#define ZFLYEMBODYANNOTATIONDIALOG_H

#include <QDialog>

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

  void setBodyId(uint64_t bodyId);

  uint64_t getBodyId() const;
  QString getComment() const;
  QString getName() const;
  QString getType() const;
  QString getStatus() const;

private:
  Ui::ZFlyEmBodyAnnotationDialog *ui;
  uint64_t m_bodyId;
};

#endif // ZFLYEMBODYANNOTATIONDIALOG_H
