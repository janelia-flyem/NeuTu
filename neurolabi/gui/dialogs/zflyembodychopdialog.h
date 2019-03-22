#ifndef ZFLYEMBODYCHOPDIALOG_H
#define ZFLYEMBODYCHOPDIALOG_H

#include <QDialog>

#include "common/neutube_def.h"

class ZAxisWidget;
class ZButtonBox;

class ZFlyEmBodyChopDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyChopDialog(QWidget *parent = 0);

  neutu::EAxis getAxis() const;

signals:

public slots:

private:
  ZAxisWidget *m_axisWidget;
  ZButtonBox *m_buttonBox;
};

#endif // ZFLYEMBODYCHOPDIALOG_H
