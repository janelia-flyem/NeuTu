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
    NeuTube::EMessageType msgType)
{
  switch (msgType) {
  case NeuTube::MSG_ERROR:
    QMessageBox::critical(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Critical);
    break;
  case NeuTube::MSG_WARNING:
    QMessageBox::warning(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Warning);
    break;
  case NeuTube::MSG_INFORMATION:
    QMessageBox::information(m_parent, title.c_str(), message.c_str());
//    m_box.setIcon(QMessageBox::Information);
    break;
  case NeuTube::MSG_DEBUG:
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
