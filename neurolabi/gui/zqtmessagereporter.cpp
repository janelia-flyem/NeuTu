#include "zqtmessagereporter.h"

ZQtMessageReporter::ZQtMessageReporter() : ZMessageReporter(), m_parent(NULL)
{
}

ZQtMessageReporter::ZQtMessageReporter(QWidget *parent) :
  ZMessageReporter(), m_parent(parent)
{
}


ZQtMessageReporter::~ZQtMessageReporter()
{

}

void ZQtMessageReporter::report(
    const std::string &title, const std::string &message,
    neutu::EMessageType msgType)
{
  switch (msgType) {
  case neutu::EMessageType::ERROR:
    QMessageBox::critical(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Critical);
    break;
  case neutu::EMessageType::WARNING:
    QMessageBox::warning(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Warning);
    break;
  case neutu::EMessageType::INFORMATION:
    QMessageBox::information(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Information);
    break;
  case neutu::EMessageType::DEBUG:
    ZMessageReporter::report(title, message, msgType);
    break;
  }

  /*
  if (msgType != NeuTube::MSG_DEBUG) {
    m_box.setWindowTitle(title.c_str());
    m_box.setText(message.c_str());
    m_box.exec();
  }
  */
}
