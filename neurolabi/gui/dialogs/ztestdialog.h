#ifndef ZTESTDIALOG_H
#define ZTESTDIALOG_H

#include <QDialog>
#include "zmessagemanager.h"
#include "zmessageprocessor.h"

namespace Ui {
class ZTestDialog;
}

class QLayout;

class ZTestDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZTestDialog(QWidget *parent = 0);
  ~ZTestDialog();

  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();

  QLayout* getMainLayout() const;

public slots:
  void testMessage();

private:
  void connectSignalSlot();

private:
  Ui::ZTestDialog *ui;
  ZMessageManager *m_messageManager;
};

#endif // ZTESTDIALOG_H
