#include "ztestdialog.h"
#include <iostream>
#include "ui_ztestdialog.h"
#include "zmessage.h"

ZTestDialog::ZTestDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZTestDialog)
{
  ui->setupUi(this);

  connectSignalSlot();

  m_messageManager = new ZMessageManager();
  m_messageManager->setProcessor(
        ZSharedPointer<MessageProcessor>(new MessageProcessor));
  m_messageManager->registerWidget(this);
}

ZTestDialog::~ZTestDialog()
{
  delete ui;
}

void ZTestDialog::connectSignalSlot()
{
  connect(ui->testPushButton, SIGNAL(clicked()), this, SLOT(testMessage()));
}

void ZTestDialog::testMessage()
{
  ZMessage message;
  message.setType(ZMessage::TYPE_INFORMATION);
  message.setBodyEntry("title", "test");
  message.setBodyEntry("body", "message test");

  m_messageManager->processMessage(&message);
}

void ZTestDialog::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
#ifdef _DEBUG_
  std::cout << "ZTestDialog::MessageProcessor::processMessage" << std::endl;
#endif
}
