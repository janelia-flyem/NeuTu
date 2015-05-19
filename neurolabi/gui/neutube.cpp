#include "neutube.h"

#include <QtGlobal>
#include <QByteArray>
#include <iostream>

#include "neutubeconfig.h"
#include "zlogmessagereporter.h"

ZMessageReporter* NeuTube::getMessageReporter()
{
  return NeutubeConfig::getInstance().getMessageReporter();
}

ZLogMessageReporter* NeuTube::getLogMessageReporter()
{
  return dynamic_cast<ZLogMessageReporter*>(getMessageReporter());
}



std::string NeuTube::getErrorFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getErrorFile();
  }

  return "";
}

std::string NeuTube::getWarnFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getWarnFile();
  }

  return "";
}

std::string NeuTube::getInfoFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getInfoFile();
  }

  return "";
}

std::string NeuTube::GetUserName()
{
#ifdef _DEBUG_2
  std::cout << qgetenv("USER").data() << std::endl;
#endif
  return qgetenv("USER").data();
}

