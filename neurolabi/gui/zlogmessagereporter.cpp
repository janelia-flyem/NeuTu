#include "zlogmessagereporter.h"
#include <fstream>
#include <iostream>
#include "tz_utilities.h"

ZLogMessageReporter::ZLogMessageReporter()
{
  m_maxFileSize = 1000000;
}

void ZLogMessageReporter::report(
    const std::string &title, const std::string &message,
    neutube::EMessageType msgType)
{
  if (msgType == neutube::MSG_DEBUG) {
    ZMessageReporter::report(std::cout, title, message, msgType);
  } else {
    switch (msgType) {
    case neutube::MSG_INFORMATION:
      if (m_infoStream.is_open()) {
        ZMessageReporter::report(m_infoStream, title, message, msgType);
      }
      break;
    case neutube::MSG_WARNING:
      if (m_warnStream.is_open()) {
        ZMessageReporter::report(m_warnStream, title, message, msgType);
      }
      break;
    case neutube::MSG_ERROR:
      if (m_errorStream.is_open()) {
        ZMessageReporter::report(m_errorStream, title, message, msgType);
      }
      break;
    default:
      break;
    }
  }
}

void ZLogMessageReporter::setInfoFile(const std::string &f)
{
   m_infoFile = f;
   if (fsize(f.c_str()) > m_maxFileSize) {
     m_infoStream.open(m_infoFile.c_str(), std::ios_base::out);
   } else {
     m_infoStream.open(m_infoFile.c_str(), std::ios_base::app);
   }
 }
