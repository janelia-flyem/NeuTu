#include "neutubeconfig.h"

#include <iostream>

#ifdef _QT_GUI_USED_
#include <QDir>
#include <QsLog.h>
#include <QDebug>
#include <QFileInfo>
#include <QFileInfoList>
#include <QString>
#include "qt/network/znetbufferreader.h"
#include "neutube.h"
#else
#include "common/neutudefs.h"
#endif

#ifdef _QT_GUI_USED_
#include "zxmldoc.h"
#endif
#include "common/utilities.h"
#include "filesystem/utilities.h"
#include "zstring.h"
#include "zlogmessagereporter.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

using namespace std;

NeutubeConfig::NeutubeConfig()
  #ifdef _QT_GUI_USED_
    : m_settings(QSettings::UserScope, "NeuTu-be")
  #endif
{
}

/*
NeutubeConfig::NeutubeConfig(const NeutubeConfig& config) :
  m_segmentationClassifThreshold(0.5), m_isSettingOn(true)
{
  UNUSED_PARAMETER(&config);
  m_messageReporter = new ZLogMessageReporter;
}
*/

NeutubeConfig::~NeutubeConfig()
{
  delete m_messageReporter;
#ifdef _QT_GUI_USED_
//  delete m_traceStream;
#endif
}

void NeutubeConfig::init(const std::string &userName)
{
  setUserName(userName);

  m_segmentationClassifThreshold = 0.5;
  m_isSettingOn = true;
  m_isStereoOn = true;
  m_autoSaveInterval = 600000;
  m_autoSaveEnabled =true;
  m_usingNativeDialog = true;
  m_usingDvidBrowseDialog = false;
  m_autoSaveMaxSwcCount = 50;

  m_messageReporter = new ZLogMessageReporter;
  setDefaultSoftwareName();
#ifdef _QT_GUI_USED_
  m_workDir = m_settings.value("workDir").toString().toStdString();
#if 0
  QString traceFilePath(getPath(NeutubeConfig::LOG_TRACE).c_str());
  QFileInfo fileInfo(traceFilePath);
  if (fileInfo.exists()) {
    QFile::rename(traceFilePath, traceFilePath + ".bk");
  }
  QFile *file = new QFile(traceFilePath);
  if (file->open(QIODevice::WriteOnly)) {
    m_traceStream = new QDebug(file);
  } else {
    m_traceStream = new QDebug(QtDebugMsg);
  }
#endif
  //  std::cout << m_settings.fileName().toStdString() << std::endl;
#endif

  m_loggingProfile = false;
  m_verboseLevel = 1;

#ifdef _QT_GUI_USED_
  if (m_settings.contains("mesh_split_thre")) {
    m_meshSplitThreshold = m_settings.value("mesh_split_thre").toInt();
  }
#endif

  updateLogDir();

}

void NeutubeConfig::setDefaultSoftwareName()
{
#if defined(_NEU3_)
  m_softwareName = "Neu3";
#else
  m_softwareName = "NeuTu";
#endif
}

void NeutubeConfig::setTestSoftwareName()
{
  setDefaultSoftwareName();
  m_softwareName += "_test";
}

void NeutubeConfig::setCliSoftwareName(const string &app)
{
  setDefaultSoftwareName();
  m_softwareName += "_" + app;
}

void NeutubeConfig::SetDefaultSoftwareName()
{
  getInstance().setDefaultSoftwareName();
}

void NeutubeConfig::SetTestSoftwareName()
{
  getInstance().setTestSoftwareName();
}

void NeutubeConfig::UpdateAutoSaveDir()
{
  getInstance().updateAutoSaveDir();
}

void NeutubeConfig::setWorkDir(const string str)
{
  if (m_workDir == m_logDir) { //Reset log dir
    m_logDir = "";
    m_logDestDir = "";
  }

  m_workDir = str;
#ifdef _QT_GUI_USED_
  getSettings().setValue("workDir", m_workDir.c_str());
#endif

  updateLogDir();
}

void NeutubeConfig::setUserName(const string &name)
{
  m_userInfo.setUserName(name);
//  m_userName = name;
}

void NeutubeConfig::UpdateUserInfo()
{
  getInstance().updateUserInfo();
}

void NeutubeConfig::updateUserInfo()
{
#ifdef _QT_GUI_USED_
#  if defined(_FLYEM_) || defined(_NEU3_)
  if (const char* user_info_entry = std::getenv("NEUTU_USER_INFO_ENTRY")) {
    ZString url(user_info_entry);
    if (!url.endsWith("/")) {
      url += "/";
    }
    url += getUserName();
    ZNetBufferReader reader;
    reader.read(url.c_str(), true);
    ZJsonObject obj;
    obj.decode(reader.getBuffer().toStdString(), false);
    if (obj.hasKey("config")) {
      ZJsonObject configObj(obj.value("config"));
      m_userInfo.setOrganization(ZJsonParser::stringValue(configObj["organization"]));
      m_userInfo.setLocation(ZJsonParser::stringValue(configObj["location"]));
    }
  }
#  endif
#endif
}

/*
void NeutubeConfig::SetUserName(const string &name)
{
  getInstance().setUserName(name);
}
*/

std::string NeutubeConfig::getUserName() const
{
  return m_userInfo.getUserName();
//  return m_userName;
}

std::string NeutubeConfig::GetUserName()
{
  return getInstance().getUserName();
}

neutu::UserInfo NeutubeConfig::getUserInfo() const
{
  return m_userInfo;
}

neutu::UserInfo NeutubeConfig::GetUserInfo()
{
  return getInstance().getUserInfo();
}

void NeutubeConfig::SetApplicationDir(const string &str)
{
  getInstance().setApplicationDir(str);
}

void NeutubeConfig::updateLogDir()
{
  if (m_logDir.empty()) {
    ZString dir = getPath(EConfigItem::WORKING_DIR);
#if defined(_QT_GUI_USED_) && defined(_FLYEM_)
    std::string candidateDir = dir;

    //Check local directry
    if (dir.startsWith("/groups/flyem/")) {
      candidateDir = "/opt/neutu_log/" + getUserName();
      if (QDir(candidateDir.c_str()).exists()) {
        m_logDir = candidateDir;
      }
    }

    //Checking shared directory
    if (m_logDir.empty()) {
      candidateDir =
          "/groups/flyem/data/neutu_log/" + getUserName();
      if (QDir(candidateDir.c_str()).exists()) {
        m_logDir = candidateDir;
      }
    }
#endif
    if (m_logDir.empty()) {
      m_logDir = dir;
      m_logDestDir = "";
    }
  }
}

void NeutubeConfig::updateAutoSaveDir()
{
#if defined(_QT_GUI_USED_)
  std::string autoSaveDir = getPath(NeutubeConfig::EConfigItem::AUTO_SAVE);
  LINFO() << "Updating autosave directory:" << autoSaveDir;
  QDir dir(autoSaveDir.c_str());
  if (dir.exists()) {
    QStringList filter;
    filter << "*.swc" << "*.SWC";
    QFileInfoList fileList =
        dir.entryInfoList(filter, QDir::NoFilter, QDir::Time);

    int maxFileCount = m_autoSaveMaxSwcCount;
    if (fileList.size() > maxFileCount) {
      int fileCount = maxFileCount;
      int originalCount = fileList.size();

      for (int i = fileCount; i < originalCount; ++i) {
        QFileInfo fileInfo = fileList.takeLast();
        QFile::remove(fileInfo.absoluteFilePath());
      }
    }
  }
#endif
}

void NeutubeConfig::operator=(const NeutubeConfig& config)
{
  UNUSED_PARAMETER(config);
}

bool NeutubeConfig::load(const std::string &filePath)
{
#ifdef _QT_GUI_USED_
#ifdef _DEBUG_
  cout << "Loading configuration ..." << endl;
#endif

  if (neutu::FileExists(filePath)) {
    ZXmlDoc doc;
    doc.parseFile(filePath);

    doc.printInfo();

    ZXmlNode node =
        doc.getRootElement().queryNode("FlyEmQASegmentationClassifier");
    if (!node.empty()) {
      m_segmentationClassifierPath =
          ZString::absolutePath(m_applicationDir, node.stringValue());
    }

    node = doc.getRootElement().queryNode("dataPath");
    m_dataPath = node.stringValue();

    node = doc.getRootElement().queryNode("docUrl");
    m_docUrl = node.stringValue();

    node = doc.getRootElement().queryNode("developPath");
    m_developPath = node.stringValue();

    if (m_dataPath.empty() && !m_developPath.empty()) {
      m_dataPath = m_developPath + "/neurolabi/data";
    }

    node = doc.getRootElement().queryNode("FlyEmQASegmentationTraining");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("test");
      m_segmentationTrainingTestPath = childNode.stringValue();
      childNode = node.queryNode("truth");
      m_segmentationTrainingTruthPath = childNode.stringValue();
      childNode = node.queryNode("feature");
      ZString str = childNode.stringValue();
      m_bodyConnectionFeature = str.toWordArray();
    }

    node = doc.getRootElement().queryNode("FlyEmQASegmentationEvaluation");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("test");
      m_segmentationEvaluationTestPath = childNode.stringValue();
      childNode = node.queryNode("truth");
      m_segmentationEvaluationTruthPath = childNode.stringValue();
      childNode = node.queryNode("threshold");
      m_segmentationClassifThreshold = childNode.doubleValue();
    }

    node = doc.getRootElement().queryNode("SwcRepository");
    if (!node.empty()) {
      m_swcRepository = node.stringValue();
    }

    node = doc.getRootElement().queryNode("Settings");
    if (!node.empty()) {
      if (node.getAttribute("status") == "off") {
        m_isSettingOn = false;
      }
    }

    node = doc.getRootElement().queryNode("Stereo");
    if (!node.empty()) {
      if (node.getAttribute("status") == "off") {
        m_isStereoOn = false;
      }
    }

    node = doc.getRootElement().queryNode("NativeFileDialog");
    if (!node.empty()) {
      if (node.getAttribute("status") == "off") {
        m_usingNativeDialog = false;
      }
    }

    node = doc.getRootElement().queryNode("DvidBrowseDialog");
    if (!node.empty()) {
      std::string attr = node.getAttribute("status");
      if (attr == "on") {
        m_usingDvidBrowseDialog = true;
      }

      if (attr == "auto") {
        const char *env = std::getenv("DVID_BROWSE_DIALOG");
        if (env) {
          if (std::string(env) == "yes") {
            m_usingDvidBrowseDialog = true;
          }
        }
      }
    }

    node = doc.getRootElement().queryNode("MainWindow");
    if (!node.empty()) {
      m_mainWindowConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Z3DWindow");
    if (!node.empty()) {
      m_z3dWindowConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Application");
    if (!node.empty()) {
      m_application = node.stringValue();
      if (m_application == "Biocytin") {
        m_mainWindowConfig.setExpandSwcWith3DWindow(true);
      }
    }

    node = doc.getRootElement().queryNode("Object_Manager");
    if (!node.empty()) {
      m_objManagerConfig.loadXmlNode(&node);
    }

    node = doc.getRootElement().queryNode("Autosave");
    if (!node.empty()) {
      ZXmlNode childNode = node.queryNode("Enabled");
      if (!childNode.empty()) {
        m_autoSaveEnabled = childNode.intValue();
      }

      childNode = node.queryNode("Interval");
      if (!childNode.empty()) {
        m_autoSaveInterval = childNode.intValue();
      }
    }

    ZLogMessageReporter *reporter =
        dynamic_cast<ZLogMessageReporter*>(m_messageReporter);
    if (reporter != NULL) {
      reporter->setInfoFile(getPath(EConfigItem::LOG_APPOUT));
      reporter->setErrorFile(getPath(EConfigItem::LOG_WARN));
      reporter->setErrorFile(getPath(EConfigItem::LOG_ERROR));
    }

    if (GET_APPLICATION_NAME == "General") {
      m_softwareName = "neuTube";
    }

    return true;
  }
#endif

  return false;
}

void NeutubeConfig::print()
{
  cout << m_dataPath << endl;
#if 0
  cout << "SWC repository: " << m_swcRepository << endl;
  cout << "Body connection classifier: " << m_segmentationClassifierPath << endl;
  cout << "Body connection training: " << endl;
  cout << "  data: " << m_segmentationTrainingTestPath << endl;
  cout << "  ground truth: " << m_segmentationTrainingTruthPath << endl;
  cout << "Body connection evaluation: " << endl;
  cout << "  data: " << m_segmentationEvaluationTestPath << endl;
  cout << "  ground truth: " << m_segmentationEvaluationTruthPath << endl;

  cout << "Bcf: ";
  for (vector<string>::const_iterator iter = m_bodyConnectionFeature.begin();
       iter != m_bodyConnectionFeature.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;
#endif
  cout << "Application dir: " << getApplicatinDir() << endl;
  cout << "Config dir: " << getConfigDir() << endl;
  cout << "Config path: " << getConfigPath() << endl;
  cout << "Autosave dir: " << getPath(EConfigItem::AUTO_SAVE) << endl;
  cout << "Autosave interval: " << m_autoSaveInterval << endl;
  cout << "Log dir: " << getPath(EConfigItem::LOG_DIR) << endl;
  cout << "Log dest dir: " << getPath(EConfigItem::LOG_DEST_DIR) << endl;
  cout << endl;
}

std::string NeutubeConfig::getPath(EConfigItem item) const
{
  switch (item) {
  case EConfigItem::DATA:
  {
    std::string dataPath;
#ifdef _QT_GUI_USED_
    if (m_settings.contains("data_dir")) {
      dataPath = m_settings.value("data_dir").toString().toStdString();
    }
#endif
    if (dataPath.empty()) {
      return m_dataPath;
    }

    return dataPath;
  }
    break;
  case EConfigItem::FLYEM_BODY_CONN_CLASSIFIER:
    return m_segmentationClassifierPath;
  case EConfigItem::FLYEM_BODY_CONN_TRAIN_DATA:
    return m_segmentationTrainingTestPath;
  case EConfigItem::FLYEM_BODY_CONN_TRAIN_TRUTH:
    return m_segmentationTrainingTruthPath;
  case EConfigItem::FLYEM_BODY_CONN_EVAL_DATA:
    return m_segmentationEvaluationTestPath;
  case EConfigItem::FLYEM_BODY_CONN_EVAL_TRUTH:
    return m_segmentationEvaluationTruthPath;
  case EConfigItem::CONFIGURE_FILE:
    return getConfigPath();
  case EConfigItem::SWC_REPOSOTARY:
    return m_swcRepository;
  case EConfigItem::AUTO_SAVE:
#ifdef _QT_GUI_USED_
      return
          QDir(getPath(EConfigItem::WORKING_DIR).c_str()).filePath("autosave").toStdString();
#else
      return ZString::fullPath(getPath(WORKING_DIR), "autosave");
#endif
  case EConfigItem::SKELETONIZATION_CONFIG:
    return getConfigDir() + ZString::FileSeparator + "json" +
        ZString::FileSeparator + "skeletonize_fib25_len40.json";
  case EConfigItem::DOCUMENT:
    return m_docUrl;
  case EConfigItem::TMP_DATA:
  {
    std::string tmpDir;
#if defined(_QT_GUI_USED_)
    std::string user = getUserName();
    tmpDir = QDir::tempPath().toStdString() + "/.neutube.z." + user;
    QDir tmpDirObj(tmpDir.c_str());
    if (!tmpDirObj.exists()) {
      if (!tmpDirObj.mkpath(tmpDir.c_str())) {
        LERROR() << "Failed to make tmp directory: " << tmpDir;
        tmpDir = "";
      }
    }
#else
    tmpDir = "/tmp";
#endif
    return tmpDir;
  }
  case EConfigItem::WORKING_DIR:
    if (m_workDir.empty()) {
#ifdef _QT_GUI_USED_
      return QDir::home().filePath(".neutube.z").toStdString();
#else
      return getApplicatinDir();
#endif
    }
    return m_workDir;
  case EConfigItem::LOG_DIR:
    return m_logDir;
  case EConfigItem::LOG_DEST_DIR:
    return m_logDestDir;
  case EConfigItem::LOG_FILE:
#ifdef _QT_GUI_USED_
  {
    QString fileName = QString((GET_SOFTWARE_NAME + "_log.txt").c_str()).toLower();

    return QDir(getPath(EConfigItem::LOG_DIR).c_str()).filePath(fileName).toStdString();
  }
#else
    return ZString::fullPath(getPath(WORKING_DIR), "log.txt");
#endif
  case EConfigItem::LOG_APPOUT:
#ifdef _QT_GUI_USED_
    return QDir(getPath(EConfigItem::WORKING_DIR).c_str()).filePath("log_appout.txt").toStdString();
#else
    return ZString::fullPath(getPath(WORKING_DIR), "log_appout.txt");
#endif
  case EConfigItem::LOG_TRACE:
#ifdef _QT_GUI_USED_
    return QDir(getPath(EConfigItem::LOG_DIR).c_str()).filePath("log_trace.txt").toStdString();
#else
    return ZString::fullPath(getPath(WORKING_DIR), "log_trace.txt");
#endif
  case EConfigItem::LOG_WARN:
#ifdef _QT_GUI_USED_
    return QDir(getPath(EConfigItem::WORKING_DIR).c_str()).filePath("log_warn.txt").toStdString();
#else
    return ZString::fullPath(getPath(WORKING_DIR), "log_warn.txt");
#endif
  case EConfigItem::LOG_ERROR:
#ifdef _QT_GUI_USED_
    return QDir(getPath(EConfigItem::WORKING_DIR).c_str()).filePath("log_error.txt").toStdString();
#else
    return ZString::fullPath(getPath(WORKING_DIR), "log_error.txt");
#endif
  case EConfigItem::NEUPRINT_AUTH:
    return ZString::fullPath(NeutubeConfig::getInstance().getPath(
          NeutubeConfig::EConfigItem::WORKING_DIR), "neuprint_auth.json");
  case EConfigItem::FLYEM_SERVICES_AUTH:
    return ZString::fullPath(NeutubeConfig::getInstance().getPath(
          NeutubeConfig::EConfigItem::WORKING_DIR), "flyem_services_auth.json");
  case EConfigItem::CONFIG_DIR:
    return ZString::fullPath(getApplicatinDir(), _CONFIG_FOLDER_);
  default:
    break;
  }

  return "";
}

std::string NeutubeConfig::getConfigDir() const
{
  return getPath(EConfigItem::CONFIG_DIR);
}
std::string NeutubeConfig::getConfigPath() const
{
  return ZString::fullPath(getConfigDir(), "config.xml");
//  return getApplicatinDir() + "/config.xml";
}
std::string NeutubeConfig::getHelpFilePath() const
{
  return ZString::fullPath(getConfigDir(), "doc", "shortcut.html");
//  return getConfigDir() + "/doc/shortcut.html";
}

NeutubeConfig::MainWindowConfig::MainWindowConfig() : m_tracingOn(true),
  m_isMarkPunctaOn(true), m_isSwcEditOn(true), m_isExpandSwcWith3DWindow(false),
  m_isProcessBinarizeOn(true), m_isMaskToSwcOn(true), m_isBinaryToSwcOn(true),
  m_isThresholdControlOn(true)
{
}

#ifdef _QT_GUI_USED_
void NeutubeConfig::MainWindowConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("tracing");
  if (!childNode.empty()) {
    enableTracing(childNode.getAttribute("status") != "off");
  } else {
    enableTracing(true);
  }

  childNode = node->queryNode("markPuncta");
  if (!childNode.empty()) {
    enableMarkPuncta(childNode.getAttribute("status") != "off");
  } else {
    enableMarkPuncta(true);
  }

  childNode = node->queryNode("swcEdit");
  if (!childNode.empty()) {
    enableSwcEdit(childNode.getAttribute("status") != "off");
  } else {
    enableSwcEdit(true);
  }

  childNode = node->queryNode("mergeImage");
  if (!childNode.empty()) {
    enableMergeImage(childNode.getAttribute("status") != "off");
  } else {
    enableMergeImage(true);
  }

  childNode = node->queryNode("Expand");
  if (!childNode.empty()) {
    ZXmlNode expandNode = childNode.queryNode("Neuron_Network");
    if (!expandNode.empty()) {
      m_isExpandNeuronNetworkOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("Tracing_Result");
    if (!expandNode.empty()) {
      m_isExpandTracingResultOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("V3D_APO");
    if (!expandNode.empty()) {
      m_isExpandV3dApoOn = (expandNode.getAttribute("status") != "off");
    }

    expandNode = childNode.queryNode("V3D_Marker");
    if (!expandNode.empty()) {
      m_isExpandV3dMarkerOn = (expandNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Process");
  if (!childNode.empty()) {
    ZXmlNode processNode = childNode.queryNode("Binarize");
    if (!processNode.empty()) {
      m_isProcessBinarizeOn = (processNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Trace");
  if (!childNode.empty()) {
    ZXmlNode traceNode = childNode.queryNode("Binary_To_Swc");
    if (!traceNode.empty()) {
      m_isBinaryToSwcOn = (traceNode.getAttribute("status") != "off");
    }

    traceNode = childNode.queryNode("Mask_To_Swc");
    if (!traceNode.empty()) {
      m_isMaskToSwcOn = (traceNode.getAttribute("status") != "off");
    }
  }

  childNode = node->queryNode("Threshold_Control");
  if (!childNode.empty()) {
    setThresholdControl(childNode.getAttribute("status") != "off");
  }
}
#endif

NeutubeConfig::Z3DWindowConfig::Z3DWindowConfig() : m_isUtilsOn(true),
  m_isVolumeOn(true), m_isGraphOn(true), m_isSwcsOn(true), m_isTubesOn(true),
  m_isPunctaOn(true), m_isMeshOn(true), m_isTensorOn(true), m_isAxisOn(true),
  m_isBackgroundOn(true)
{
}
#ifdef _QT_GUI_USED_
void NeutubeConfig::Z3DWindowConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Utils");
  if (!childNode.empty()) {
    enableUtils(childNode.getAttribute("status") != "off");
  } else {
    enableUtils(true);
  }

  childNode = node->queryNode("Volume");
  if (!childNode.empty()) {
    enableVolume(childNode.getAttribute("status") != "off");
  } else {
    enableVolume(true);
  }

  childNode = node->queryNode("Graph");
  if (!childNode.empty()) {
    enableGraph(childNode.getAttribute("status") != "off");
    m_graphTabConfig.loadXmlNode(&childNode);
  } else {
    enableGraph(true);
  }

  childNode = node->queryNode("Swcs");
  if (!childNode.empty()) {
    enableSwcs(childNode.getAttribute("status") != "off");
    m_swcTabConfig.loadXmlNode(&childNode);
  } else {
    enableSwcs(true);
  }

  childNode = node->queryNode("Tubes");
  if (!childNode.empty()) {
    enableTubes(childNode.getAttribute("status") != "off");
  } else {
    enableTubes(true);
  }

  childNode = node->queryNode("Puncta");
  if (!childNode.empty()) {
    enablePuncta(childNode.getAttribute("status") != "off");
  } else {
    enablePuncta(true);
  }

  childNode = node->queryNode("Tensor");
  if (!childNode.empty()) {
    enableTensor(childNode.getAttribute("status") != "off");
  } else {
    enableTensor(true);
  }

  childNode = node->queryNode("Mesh");
  if (!childNode.empty()) {
    enableMesh(childNode.getAttribute("status") != "off");
  } else {
    enableMesh(true);
  }

  childNode = node->queryNode("Background");
  if (!childNode.empty()) {
    enableBackground(childNode.getAttribute("status") != "off");
  } else {
    enableBackground(true);
  }

  childNode = node->queryNode("Axis");
  if (!childNode.empty()) {
    enableAxis(childNode.getAttribute("status") != "off");
  } else {
    enableAxis(true);
  }
}
#endif

NeutubeConfig::Z3DWindowConfig::GraphTabConfig::GraphTabConfig() :
  m_isVisible(true), m_opacity(1.0)
{
}

#ifdef _QT_GUI_USED_
void NeutubeConfig::Z3DWindowConfig::GraphTabConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Visible");
  if (!childNode.empty()) {
    m_isVisible = childNode.intValue() > 0;
  }

  childNode = node->queryNode("Opacity");
  if (!childNode.empty()) {
    m_opacity = childNode.doubleValue();
  }
}
#endif

NeutubeConfig::Z3DWindowConfig::SwcTabConfig::SwcTabConfig() : m_primitive("Normal"),
  m_colorMode("Branch Type"), m_zscale(1.0)
{
}

#ifdef _QT_GUI_USED_
void NeutubeConfig::Z3DWindowConfig::SwcTabConfig::loadXmlNode(const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Primitive");
  if (!childNode.empty()) {
    m_primitive = childNode.stringValue();
  }

  childNode = node->queryNode("Color_Mode");
  if (!childNode.empty()) {
    m_colorMode = childNode.stringValue();
  }

  childNode = node->queryNode("ZScale");
  if (!childNode.empty()) {
    m_zscale = childNode.doubleValue();
  }
}
#endif

NeutubeConfig::ObjManagerConfig::ObjManagerConfig() :
  m_isSwcOn(true), m_isSwcNodeOn(true), m_isPunctaOn(true)
{

}
#ifdef _QT_GUI_USED_
void NeutubeConfig::ObjManagerConfig::loadXmlNode(
    const ZXmlNode *node)
{
  ZXmlNode childNode = node->queryNode("Swc");
  if (!childNode.empty()) {
    m_isSwcOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isSwcOn = true;
  }

  childNode = node->queryNode("Swc_Node");
  if (!childNode.empty()) {
    m_isSwcNodeOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isSwcNodeOn = true;
  }

  childNode = node->queryNode("Puncta");
  if (!childNode.empty()) {
    m_isPunctaOn = (childNode.getAttribute("status") != "off");
  } else {
    m_isPunctaOn = true;
  }
}
#endif

void NeutubeConfig::configure(const ZJsonObject &obj)
{
  if (obj.hasKey("profiling")) {
    m_loggingProfile = ZJsonParser::booleanValue(obj["profiling"]);
  }

  if (obj.hasKey("verbose")) {
    m_verboseLevel = ZJsonParser::integerValue(obj["verbose"]);
  }
}

void NeutubeConfig::enableProfileLogging(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("profiling", on);
#else
  m_loggingProfile = on;
#endif
}

void NeutubeConfig::setVerboseLevel(int level)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("verbose", level);
#else
  m_verboseLevel = level;
#endif
}

int NeutubeConfig::getVerboseLevel() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("verbose")) {
    return m_settings.value("verbose").toInt();
  }
#endif

  return m_verboseLevel;
}

bool NeutubeConfig::parallelTileFetching() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("parallel_tile")) {
    return m_settings.value("parallel_tile").toBool();
  }
#endif

  return true;
}

bool NeutubeConfig::lowtisPrefetching() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("lowtis_prefetching")) {
    return m_settings.value("lowtis_prefetching").toBool();
  }
#endif

  return false;
}

void NeutubeConfig::setParallelTileFetching(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("parallel_tile", on);
#endif
}

void NeutubeConfig::enableLowtisPrefetching(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("lowtis_prefetching", on);
#endif
}

void NeutubeConfig::SetParallelTileFetching(bool on)
{
  getInstance().setParallelTileFetching(on);
}

void NeutubeConfig::EnableLowtisPrefetching(bool on)
{
  getInstance().enableLowtisPrefetching(on);
}

bool NeutubeConfig::LowtisPrefetching()
{
  return getInstance().lowtisPrefetching();
}

bool NeutubeConfig::namingSynapse() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("naming_synapse")) {
    return m_settings.value("naming_synapse").toBool();
  }
#endif

  return false;
}

bool NeutubeConfig::namingPsd() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("naming_psd")) {
    return m_settings.value("naming_psd").toBool();
  }
#endif

  return false;
}

void NeutubeConfig::setNamingSynapse(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("naming_synapse", on);
#endif
}

void NeutubeConfig::setNamingPsd(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("naming_psd", on);
#endif
}

bool NeutubeConfig::loggingProfile() const
{
#ifdef _QT_GUI_USED_
  if (m_settings.contains("profiling")) {
    return m_settings.value("profiling").toBool();
  }
#endif

  return m_loggingProfile;
}

void NeutubeConfig::enableAutoStatusCheck(bool on)
{
#ifdef _QT_GUI_USED_
  m_settings.setValue("auto_status", on);
#endif
}

bool NeutubeConfig::autoStatusCheck() const
{
#if _QT_GUI_USED_
  if (m_settings.contains("auto_status")) {
    return m_settings.value("auto_status").toBool();
  }
#endif

  return true;
}

void NeutubeConfig::EnableAutoStatusCheck(bool on)
{
  getInstance().enableAutoStatusCheck(on);
}

bool NeutubeConfig::AutoStatusCheck()
{
  return getInstance().autoStatusCheck();
}

void NeutubeConfig::EnableProfileLogging(bool on)
{
  getInstance().enableProfileLogging(on);
}

bool NeutubeConfig::LoggingProfile()
{
  return getInstance().loggingProfile();
}

void NeutubeConfig::setAdvancedMode(bool on)
{
  m_advancedMode = on;
}

bool NeutubeConfig::isAdvancedMode() const
{
  return m_advancedMode;
}

void NeutubeConfig::SetAdvancedMode(bool on)
{
  getInstance().setAdvancedMode(on);
}

bool NeutubeConfig::IsAdvancedMode()
{
  return getInstance().isAdvancedMode();
}

void NeutubeConfig::set3DCrossWidth(double w)
{
  m_3dcrossWidth = w;
#ifdef _QT_GUI_USED_
  m_settings.setValue("3d_cross_width", w);
#endif
}

double NeutubeConfig::get3DCrossWidth() const
{
  return m_3dcrossWidth;
}

void NeutubeConfig::Set3DCrossWidth(int w)
{
  getInstance().set3DCrossWidth(w);
}

int NeutubeConfig::Get3DCrossWidth()
{
  return getInstance().get3DCrossWidth();
}

void NeutubeConfig::setMeshSplitThreshold(size_t thre)
{
  m_meshSplitThreshold = thre;
#ifdef _QT_GUI_USED_
  m_settings.setValue("mesh_split_thre", int(thre));
#endif
}

size_t NeutubeConfig::getMeshSplitThreshold() const
{
  return m_meshSplitThreshold;
}

void NeutubeConfig::SetMeshSplitThreshold(size_t thre)
{
  getInstance().setMeshSplitThreshold(thre);
}

size_t NeutubeConfig::GetMeshSplitThreshold()
{
  return getInstance().getMeshSplitThreshold();
}

int NeutubeConfig::GetVerboseLevel()
{
  return getInstance().getVerboseLevel();
}

void NeutubeConfig::SetVerboseLevel(int level)
{
  getInstance().setVerboseLevel(level);
}

bool NeutubeConfig::ParallelTileFetching()
{
  return getInstance().parallelTileFetching();
}

void NeutubeConfig::Configure(const ZJsonObject &obj)
{
  getInstance().configure(obj);
}

std::string NeutubeConfig::GetSoftwareName()
{
  return getInstance().getSoftwareName();
}


#ifdef _QT_GUI_USED_
QString NeutubeConfig::GetFlyEmConfigPath()
{
  QString path = GetSettings().value("flyem_config").toString();

  return path;
}

void NeutubeConfig::SetFlyEmConfigPath(const QString &path)
{
  GetSettings().setValue("flyem_config", path);
}

void NeutubeConfig::UseDefaultFlyEmConfig(bool on)
{
  GetSettings().setValue("default_flyem_config", on);
}

void NeutubeConfig::UseDefaultNeuTuServer(bool on)
{
  GetSettings().setValue("default_neutu_server", on);
}

bool NeutubeConfig::UsingDefaultNeuTuServer()
{
  if (GetSettings().contains("default_neutu_server")) {
    return GetSettings().value("default_neutu_server").toBool();
  }

  return true;
}

void NeutubeConfig::UseDefaultTaskServer(bool on)
{
  GetSettings().setValue("default_task_server", on);
}

bool NeutubeConfig::UsingDefaultTaskServer()
{
  if (GetSettings().contains("default_task_server")) {
    return GetSettings().value("default_task_server").toBool();
  }

  return true;
}

QString NeutubeConfig::GetNeuTuServer()
{
  return GetSettings().value("neutu_server").toString();
}

QString NeutubeConfig::GetTaskServer()
{
  return GetSettings().value("task_server").toString();
}

bool NeutubeConfig::UsingDefaultFlyemConfig()
{
  if (GetSettings().contains("default_flyem_config")) {
    return GetSettings().value("default_flyem_config").toBool();
  }

  return true;
}

bool NeutubeConfig::NamingSynapse()
{
  return getInstance().namingSynapse();
}

bool NeutubeConfig::NamingPsd()
{
  return getInstance().namingPsd();
}

void NeutubeConfig::SetNamingSynapse(bool on)
{
  getInstance().setNamingSynapse(on);
}

void NeutubeConfig::SetNamingPsd(bool on)
{
  getInstance().setNamingPsd(on);
}

void NeutubeConfig::SetNeuTuServer(const QString &path)
{
  if (path.isEmpty()) {
    GetSettings().setValue("neutu_server", ":");
  } else {
    GetSettings().setValue("neutu_server", path);
  }
}

void NeutubeConfig::SetTaskServer(const QString &path)
{
  GetSettings().setValue("task_server", path);
}

void NeutubeConfig::SetDataDir(const QString &dataDir)
{
  GetSettings().setValue("data_dir", dataDir);
}

#endif



