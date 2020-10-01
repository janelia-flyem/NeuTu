#include "neutube.h"

#include <string>

#include <QtGlobal>
#include <QByteArray>
#include <QMetaType>
#include <iostream>
#include <QProcess>
#include <QSet>

#include "neutubeconfig.h"
#include "zlogmessagereporter.h"
#include "dvid/zdvidtarget.h"
#include "zstackdocptr.h"
#include "zwidgetmessage.h"
#include "swctreenode.h"
#include "zstackobjectrole.h"
#include "zstackobjectinfo.h"
#include "zstackviewparam.h"
#include "zarbsliceviewparam.h"
#include "data3d/zsliceviewtransform.h"

void neutu::RegisterMetaType()
{
  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<std::string>("std::string");
  qRegisterMetaType<ZJsonValue>("ZJsonValue");
  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");
  qRegisterMetaType<ZStackDocPtr>("ZStackDocPtr");
  qRegisterMetaType<neutu::data3d::ETarget>("neutu::data3d::ETarget");
  qRegisterMetaType<QSet<neutu::data3d::ETarget> >("QSet<neutu::data3d::ETarget>");
  qRegisterMetaType<QList<Swc_Tree_Node*> >("QList<Swc_Tree_Node*>");
  qRegisterMetaType<ZWidgetMessage>("ZWidgetMessage");
  qRegisterMetaType<std::set<uint64_t> >("std::set<uint64_t>");
  qRegisterMetaType<std::vector<int>>("std::vector<int>");
  qRegisterMetaType<QSet<uint64_t> >("QSet<uint64_t>");
  qRegisterMetaType<neutu::EBodySplitMode>("neutu::EBodySplitMode");
  qRegisterMetaType<ZStackObjectInfo>("ZStackObjectInfo");
  qRegisterMetaType<ZStackObjectInfoSet>("ZStackObjectInfoSet");
  qRegisterMetaType<ZStackViewParam>("ZStackViewParam");
  qRegisterMetaType<ZArbSliceViewParam>("ZArbSliceViewParam");
  qRegisterMetaType<ZIntPoint>("ZIntPoint");
  qRegisterMetaType<ZSliceViewTransform>("ZSliceViewTransform");
}

ZMessageReporter* neutu::getMessageReporter()
{
  return NeutubeConfig::getInstance().getMessageReporter();
}

ZLogMessageReporter* neutu::getLogMessageReporter()
{
  return dynamic_cast<ZLogMessageReporter*>(getMessageReporter());
}


std::string neutu::getErrorFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getErrorFile();
  }

  return "";
}

std::string neutu::getWarnFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getWarnFile();
  }

  return "";
}

std::string neutu::getInfoFile()
{
  if (getLogMessageReporter() != NULL) {
    return getLogMessageReporter()->getInfoFile();
  }

  return "";
}

std::string neutu::GetCurrentUserName()
{
//  if (NeutubeConfig::GetUserName().empty()) {
//    NeutubeConfig::SetUserName(qgetenv("USER").toStdString());
//  }

#ifdef _DEBUG_2
  std::cout << "User name: " << NeutubeConfig::GetUserName() << std::endl;
#endif

  return NeutubeConfig::GetUserName();
}

bool neutu::IsAdminUser()
{
#if defined(_FLYEM_)
  if (neutu::GetCurrentUserName() == "takemuras" ||
      neutu::GetCurrentUserName() == "shinomiyak" ||
      neutu::GetCurrentUserName() == "jah") {
    return true;
  }
#endif

  return neutu::GetCurrentUserName() == "zhaot";
}

QFileDialog::Options neutu::GetFileDialogOption()
{
  if (!NeutubeConfig::getInstance().usingNativeDialog()) {
    return QFileDialog::DontUseNativeDialog;
  }

  return 0;
}

QString neutu::GetLastFilePath()
{
  return NeutubeConfig::GetSettings().value("lastPath").toString();
}

QString neutu::GetFilePath(const QUrl &url)
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
