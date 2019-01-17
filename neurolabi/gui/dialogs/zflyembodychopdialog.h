#ifndef ZFLYEMBODYCHOPDIALOG_H
#define ZFLYEMBODYCHOPDIALOG_H

#include <QDialog>

#include "core/neutube_def.h"

class ZAxisWidget;
class ZButtonBox;

class ZFlyEmBodyChopDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyChopDialog(QWidget *parent = 0);

  neutube::EAxis getAxis() const;

signals:

public slots:

private:
  ZAxisWidget *m_axisWidget;
  ZButtonBox *m_buttonBox;
};

#endif // ZFLYEMBODYCHOPDIALOG_H
