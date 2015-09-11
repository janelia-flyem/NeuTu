#ifndef NEUTUBE_H
#define NEUTUBE_H

#include <string>
#include "neutube_def.h"

class ZMessageReporter;
class ZLogMessageReporter;

namespace NeuTube {

ZMessageReporter *getMessageReporter();
ZLogMessageReporter* getLogMessageReporter();

std::string getErrorFile();
std::string getWarnFile();
std::string getInfoFile();


std::string GetCurrentUserName();
bool IsAdminUser();
}



#endif // NEUTUBE_H
