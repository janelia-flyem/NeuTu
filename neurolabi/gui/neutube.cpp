#include "neutube.h"

#include <QtGlobal>
#include <QByteArray>
#include <QMetaType>
#include <iostream>
#include <QProcess>
#include <QSet>

#include "neutubeconfig.h"
#include "zlogmessagereporter.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoc.h"
#include "zwidgetmessage.h"

void neutube::RegisterMetaType()
{
  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<ZJsonValue>("ZJsonValue");
  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");
  qRegisterMetaType<ZStackDocPtr>("ZStackDocPtr");
  qRegisterMetaType<QSet<ZStackObject::ETarget> >("QSet<ZStackObject::ETarget>");
  qRegisterMetaType<QList<Swc_Tree_Node*> >("QList<Swc_Tree_Node*>");
  qRegisterMetaType<ZWidgetMessage>("ZWidgetMessage");
  qRegisterMetaType<std::set<uint64_t> >("std::set<uint64_t>");
  qRegisterMetaType<QSet<uint64_t> >("QSet<uint64_t>");
  qRegisterMetaType<flyem::EBodySplitMode>("flyem::EBodySplitMode");
}

ZMessageReporter* neutube::getMessageReporter()
{
  return NeutubeConfig::getInstance().getMessageReporter();
}

ZLogMessageReporter* neutube::getLogMessageReporter()
{
  return dynamic_cast<ZLogMessageReporter*>(getMessageReporter());
}


std::string neutube::getErrorFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getErrorFile();
  }

  return "";
}

std::string neutube::getWarnFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getWarnFile();
  }

  return "";
}

std::string neutube::getInfoFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getInfoFile();
  }

  return "";
}

std::string neutube::GetCurrentUserName()
{
#ifdef _DEBUG_
  std::cout << "User name: " << qgetenv("USER").data() << std::endl;
#endif
  std::string userName = qgetenv("USER").data();


  if (userName == "zhaot") { //temporary hack
//    userName = "ogundeyio";
  }

  return userName;
}

bool neutube::IsAdminUser()
{
#if defined(_FLYEM_)
  if (neutube::GetCurrentUserName() == "takemuras" ||
      neutube::GetCurrentUserName() == "shinomiyak" ||
      neutube::GetCurrentUserName() == "jah") {
    return true;
  }
#endif

  return neutube::GetCurrentUserName() == "zhaot";
}

QFileDialog::Options neutube::GetFileDialogOption()
{
  if (!NeutubeConfig::getInstance().usingNativeDialog()) {
    return QFileDialog::DontUseNativeDialog;
  }

  return 0;
}

QString neutube::GetLastFilePath()
{
  return NeutubeConfig::GetSettings().value("lastPath").toString();
}

QString neutube::GetFilePath(const QUrl &url)
{
  QString filePath;

#ifdef _WIN32
  // remove leading slash
  if (url.path().at(0) == QChar('/')) {
    filePath = url.path().mid(1)
  } else {
    filePath = url.path();
  }
#else
#  if defined(__APPLE__)
  QString localFile = url.toLocalFile();
  if ( localFile.startsWith("/.file/id=") ) {
    QProcess process;
    QStringList argList =
        QStringList() << "-e get posix path of my posix file \"" +
                         localFile + "\" -- kthx. bai";
    process.start("osascript", argList);
    process.waitForFinished();
//      QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray byteArray = process.readAllStandardOutput().trimmed();
    localFile = QString::fromUtf8(byteArray);
//      qDebug() << localFile;
    QFileInfo fileInfo(localFile);
    if (!localFile.isEmpty() && fileInfo.exists()) {
      filePath = localFile;
    }
  } else {
    filePath = url.path();
  }
#  else
 filePath = url.path();
#  endif
#endif

  return filePath;
}
