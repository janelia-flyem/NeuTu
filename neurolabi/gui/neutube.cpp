#include "neutube.h"

#include <QtGlobal>
#include <QByteArray>
#include <QMetaType>
#include <iostream>
#include <QProcess>

#include "neutubeconfig.h"
#include "zlogmessagereporter.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoc.h"
#include "zwidgetmessage.h"

void NeuTube::RegisterMetaType()
{
  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<ZJsonValue>("ZJsonValue");
  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");
  qRegisterMetaType<ZStackDocPtr>("ZStackDocPtr");
  qRegisterMetaType<QSet<ZStackObject::ETarget> >("QSet<ZStackObject::ETarget>");
  qRegisterMetaType<QList<Swc_Tree_Node*> >("QList<Swc_Tree_Node*>");
  qRegisterMetaType<ZWidgetMessage>("ZWidgetMessage");
  qRegisterMetaType<std::set<uint64_t> >("std::set<uint64_t>");
}

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

  /*
  if (userName == "zhaot") { //temporary hack
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

QFileDialog::Options NeuTube::GetFileDialogOption()
{
  if (!NeutubeConfig::getInstance().usingNativeDialog()) {
    return QFileDialog::DontUseNativeDialog;
  }

  return 0;
}

QString NeuTube::GetLastFilePath()
{
  return NeutubeConfig::GetSettings().value("lastPath").toString();
}

QString NeuTube::GetFilePath(const QUrl &url)
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
