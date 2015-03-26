#include "ztestdialog.h"
#include <iostream>
#include "ui_ztestdialog.h"
#include "zmessage.h"
#include "dvid/zdvidversionmodel.h"

ZTestDialog::ZTestDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZTestDialog),
  m_messageManager(NULL)
{
  ui->setupUi(this);

  connectSignalSlot();

  ZDvidVersionModel *model = new ZDvidVersionModel(this);

  //model->getDag().setRoot("root");
  //model->getDag().addNode("v1", "root");

  ui->treeView->setModel(model);

  model->setRoot("root");
  model->addNode("v1", "root");
  model->addNode("v2", "root");
  model->addNode("v3", "v1");
  model->addNode("v3", "v2");

  enableMessageManager();
}

ZTestDialog::~ZTestDialog()
{
  delete ui;
}

void ZTestDialog::connectSignalSlot()
{
  connect(ui->testPushButton, SIGNAL(clicked()), this, SLOT(testMessage()));
}

void ZTestDialog::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }
}

void ZTestDialog::testMessage()
{
  ZMessage message;
  message.setType(ZMessage::TYPE_INFORMATION);
  message.setBodyEntry("title", "test");
  message.setBodyEntry("body", "message test");

  m_messageManager->processMessage(&message, true);
}

void ZTestDialog::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
#ifdef _DEBUG_
  std::cout << "ZTestDialog::MessageProcessor::processMessage" << std::endl;
#endif
}

QLayout* ZTestDialog::getMainLayout() const
{
  return ui->verticalLayout;
}
