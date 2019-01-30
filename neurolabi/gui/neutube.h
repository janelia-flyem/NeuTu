#ifndef NEUTUBE_H
#define NEUTUBE_H

#include <QFileDialog>
#include <string>
#include "common/neutube_def.h"

class ZMessageReporter;
class ZLogMessageReporter;

namespace neutube {

ZMessageReporter *getMessageReporter();
ZLogMessageReporter* getLogMessageReporter();

std::string getErrorFile();
std::string getWarnFile();
std::string getInfoFile();


std::string GetCurrentUserName();
bool IsAdminUser();

QFileDialog::Options GetFileDialogOption();
QString GetLastFilePath();

void RegisterMetaType();

QString GetFilePath(const QUrl &url);
}



#endif // NEUTUBE_H
