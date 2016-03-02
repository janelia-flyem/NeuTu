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

std::string NeuTube::GetCurrentUserName()
{
#ifdef _DEBUG_2
  std::cout << qgetenv("USER").data() << std::endl;
#endif
  std::string userName = qgetenv("USER").data();

//  userName = "changl";
  /*
  if (userName == "plazas") { //temporary hack
    userName = "takemuras";
  }
  */

  return userName;
}

bool NeuTube::IsAdminUser()
{
#if defined(_FLYEM_)
  if (NeuTube::GetCurrentUserName() == "takemuras" ||
      NeuTube::GetCurrentUserName() == "shinomiyak" ||
      NeuTube::GetCurrentUserName() == "jah") {
    return true;
  }
#endif

  return NeuTube::GetCurrentUserName() == "zhaot";
}

