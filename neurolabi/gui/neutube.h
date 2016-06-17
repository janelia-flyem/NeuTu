#ifndef NEUTUBE_H
#define NEUTUBE_H

#include <QFileDialog>
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

QFileDialog::Options GetFileDialogOption();
QString GetLastFilePath();
}



#endif // NEUTUBE_H
