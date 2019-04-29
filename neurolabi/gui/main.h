#ifndef MAIN_H
#define MAIN_H

#ifdef _QT5_
#include <QSurfaceFormat>
#endif
#include <QApplication>
#include <QFileInfo>

#include "neutube.h"
#include "zneurontracer.h"
#include "logging/zqslog.h"
#include "neutubeconfig.h"
#include "zjsonparser.h"
#include "zcommandline.h"
#include "zneurontracerconfig.h"
#include "flyem/zglobaldvidrepo.h"

#if 0
#ifdef _QT5_
#include <QSurfaceFormat>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  switch (type) {
  case QtDebugMsg:
    LDEBUGF(context.file, context.line, context.function) << msg;
    break;
  case QtWarningMsg:
    LWARNF(context.file, context.line, context.function) << msg;
    break;
  case QtCriticalMsg:
    LERRORF(context.file, context.line, context.function) << msg;
    break;
  case QtFatalMsg:
    LFATALF(context.file, context.line, context.function) << msg;
    abort();
  default:
    break;
  }
}
#else
void myMessageOutput(QtMsgType type, const char *msg)
{
  switch (type) {
  case QtDebugMsg:
    LDEBUG_NLN() << msg;
    break;
  case QtWarningMsg:
    LWARN_NLN() << msg;
    break;
  case QtCriticalMsg:
    LERROR_NLN() << msg;
    break;
  case QtFatalMsg:
    LFATAL_NLN() << msg;
    abort();
  }
}
#endif    // qt version > 5.0.0
#endif

namespace neutu {
static std::string UserName;
}

namespace {

void sync_log_dir(const std::string &srcDir, const std::string &destDir)
{
    if (!srcDir.empty() && !destDir.empty() && srcDir != destDir) {
        QDir dir(srcDir.c_str());
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList infoList =
                dir.entryInfoList(QStringList() << "*.txt.*" << "*.txt");

        foreach (const QFileInfo &info, infoList) {
            QString command =
                    ("rsync -uv " + srcDir + "/" + info.fileName().toStdString() +
                     " " + destDir + "/").c_str();
            std::cout << command.toStdString() << std::endl;
            QProcess process;
            process.start(command);
            process.waitForFinished(-1);
            QString errorOutput = process.readAllStandardError();
            QString standardOutout = process.readAllStandardOutput();
            std::cout << errorOutput.toStdString() << std::endl;
            std::cout << standardOutout.toStdString() << std::endl;
        }
    }
}

#ifdef _FLYEM_
void SetFlyEmConfigpath(
    const QString &rootConfigPath, const ZJsonObject configObj)
{
  ZJsonArray defaultConfigCandidate(configObj.value("flyem"));

  QFileInfo rootConfigFileInfo(rootConfigPath);
  QFileInfo configFileInfo;
//  QDir appDir((GET_APPLICATION_DIR).c_str());
  QDir rootDir = rootConfigFileInfo.absoluteDir();

  for (size_t i = 0; i < defaultConfigCandidate.size(); ++i) {
    std::string path = ZJsonParser::stringValue(configObj["flyem"], i);
    configFileInfo.setFile(rootDir, path.c_str());
    if (configFileInfo.exists()) {
      break;
    }
  }

  if (configFileInfo.exists()) {
    GET_FLYEM_CONFIG.setDefaultConfigPath(
          configFileInfo.absoluteFilePath().toStdString());
  }

  QString flyemConfigPath = NeutubeConfig::GetFlyEmConfigPath();
  GET_FLYEM_CONFIG.setConfigPath(flyemConfigPath.toStdString());
}
#endif

#ifdef _FLYEM_
void LoadFlyEmConfig(
    const QString &configPath, NeutubeConfig &/*config*/, bool usingConfig)
{
  ZJsonObject configObj;
  if (!configPath.isEmpty()) {
    configObj.load(configPath.toStdString());
  }

  GET_FLYEM_CONFIG.useDefaultConfig(NeutubeConfig::UsingDefaultFlyemConfig());
  GET_FLYEM_CONFIG.useDefaultNeuTuServer(NeutubeConfig::UsingDefaultNeuTuServer());
  GET_FLYEM_CONFIG.useDefaultTaskServer(NeutubeConfig::UsingDefaultTaskServer());

  SetFlyEmConfigpath(configPath, configObj);

  GET_FLYEM_CONFIG.loadConfig();
  GET_FLYEM_CONFIG.loadUserSettings();

  //Settings provided by a more general source
  if (usingConfig) {
#ifdef _DEBUG_2
    std::cout << "NeuTu server: " << config.GetNeuTuServer().toStdString() << std::endl;
#endif

    if (GET_FLYEM_CONFIG.hasDefaultNeuTuServer() == false) {
      std::string neutuServer = ZJsonParser::stringValue(configObj["neutu_server"]);
      if (!neutuServer.empty()) {
        GET_FLYEM_CONFIG.setCustomNeuTuServer(neutuServer);
        //        GET_FLYEM_CONFIG.setDefaultNeuTuServer(neutuServer.toStdString());
      }
    }

#ifdef _DEBUG_2
    GET_FLYEM_CONFIG.setServer("neutuse:http://127.0.0.1:5000");
#endif

    if (GET_FLYEM_CONFIG.hasDefaultTaskServer() == false) {
      std::string taskServer = ZJsonParser::stringValue(configObj["task_server"]);
      if (!taskServer.empty()) {
        GET_FLYEM_CONFIG.setCustomTaskServer(taskServer);
      }
    }
  }
}
#endif

void init_log()
{
  // init the logging mechanism
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  const QString sLogPath(
        NeutubeConfig::getInstance().getPath(
          NeutubeConfig::EConfigItem::LOG_FILE).c_str());
  const QString traceLogPath(
        NeutubeConfig::getInstance().getPath(
          NeutubeConfig::EConfigItem::LOG_TRACE).c_str());

#ifdef _FLYEM_
  int maxLogCount = 100;
#else
  int maxLogCount = 10;
#endif

  QsLogging::DestinationPtr fileDestination(
        QsLogging::DestinationFactory::MakeFileDestination(
          sLogPath, QsLogging::EnableLogRotation,
          QsLogging::MaxSizeBytes(5e7), QsLogging::MaxOldLogCount(maxLogCount)));
  QsLogging::DestinationPtr traceFileDestination(
        QsLogging::DestinationFactory::MakeFileDestination(
          traceLogPath, QsLogging::EnableLogRotation,
          QsLogging::MaxSizeBytes(2e7), QsLogging::MaxOldLogCount(10),
          QsLogging::TraceLevel));
  QsLogging::DestinationPtr debugDestination(
        QsLogging::DestinationFactory::MakeDebugOutputDestination());
  logger.addDestination(debugDestination);
  logger.addDestination(traceFileDestination);
  logger.addDestination(fileDestination);
#if defined _DEBUG_
  logger.setLoggingLevel(QsLogging::DebugLevel);
#else
  logger.setLoggingLevel(QsLogging::InfoLevel);
#endif

  if (NeutubeConfig::GetVerboseLevel() >= 5) {
    logger.setLoggingLevel(QsLogging::TraceLevel);
  }
}

struct MainConfig {
  bool isGuiEnabled() const
  {
    return !(debugging || runCommandLine);
  }

  bool debugging = false;
  bool unitTest = false;
  bool runCommandLine = false;

  bool guiEnabled = true;
  bool advanced = false;
  bool showingVersion = false;
  bool autoTestingTask = false;

  std::string userName;
  QStringList fileList;
  QString configPath;
};

MainConfig get_program_config(int argc, char *argv[])
{
  MainConfig config;
  if (argc > 1) {
    if (QString(argv[1]).startsWith("user:")) {
      config.userName = std::string(argv[1]).substr(5);
    }
  }
  if (config.userName.empty()) {
    config.userName = qgetenv("USER").toStdString();
  }

  if (qgetenv("NEUTU_ADVANCED").toInt() == 1) {
    config.advanced = true;
  }

  if (qgetenv("AUTO_TESTING_TASK").toInt() == 1) {
    config.autoTestingTask = true;
  }

  if (argc > 1) {
    if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
      config.showingVersion = true;
    }

    if (strcmp(argv[1], "d") == 0) {
      config.debugging = true;
    }

    if (strcmp(argv[1], "a") == 0) {
      config.advanced = true;
    }

    if (strcmp(argv[1], "--command") == 0) {
      config.runCommandLine = true;
    }

    if (strcmp(argv[1], "u") == 0 || QString(argv[1]).startsWith("--gtest")) {
      config.unitTest = true;
      config.debugging = true;
    }

    if (strcmp(argv[1], "--load") == 0) {
      for (int i = 2; i < argc; ++i) {
        config.fileList << argv[i];
      }
    }

    if (QString(argv[1]).endsWith(".json")) {
      config.configPath = argv[1];
    }
  }
//  if (config.debugging || config.runCommandLine) {
//    config.guiEnabled = false;
//  }

  return config;
}

int run_command_line(int argc, char *argv[])
{
#if defined(_FLYEM_)
  NeutubeConfig &config = NeutubeConfig::getInstance();
  QFileInfo fileInfo(argv[0]);
  std::string appDir = fileInfo.absoluteDir().absolutePath().toStdString();
  config.setApplicationDir(appDir);
  LoadFlyEmConfig("", config, false);
#endif

  init_log();

  ZCommandLine cmd;
  return cmd.run(argc, argv);
}

void init_gui()
{
#ifdef _QT5_
  QSurfaceFormat format;
#if defined(__APPLE__) && defined(_USE_CORE_PROFILE_)
  format.setVersion(3, 2);
  format.setProfile(QSurfaceFormat::CoreProfile);
#endif
  //format.setStereo(true);
  QSurfaceFormat::setDefaultFormat(format);

  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
#endif
  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
}

void configure(MainConfig &mainConfig)
{
  //load config
  NeutubeConfig &config = NeutubeConfig::getInstance();
  config.setAdvancedMode(mainConfig.advanced);

  std::cout << QApplication::applicationDirPath().toStdString() << std::endl;
  config.setApplicationDir(QApplication::applicationDirPath().toStdString());

  if (config.load(config.getConfigPath()) == false) {
    std::cout << "Unable to load configuration: "
              << config.getConfigPath() << std::endl;
  }

  if (mainConfig.configPath.isEmpty()) {
    mainConfig.configPath =
        QFileInfo(QDir((GET_CONFIG_DIR + "/json").c_str()), "config.json").
        absoluteFilePath();
  }

#ifdef _FLYEM_
  LoadFlyEmConfig(mainConfig.configPath, config, true);
  /*
  if (mainConfig.isGuiEnabled()) {
    GET_FLYEM_CONFIG.activateNeuTuServer();
  }
  */

  ZGlobalDvidRepo::GetInstance().init();
#endif

  if (!mainConfig.runCommandLine) { //Command line mode takes care of configuration independently
#if !defined(_FLYEM_)
    ZNeuronTracerConfig &tracingConfig = ZNeuronTracerConfig::getInstance();
    tracingConfig.load(config.getConfigDir() + "/json/trace_config.json");

    if (GET_APPLICATION_NAME == "Biocytin") {
      tracingConfig.load(
            config.getConfigDir() + "/json/trace_config_biocytin.json");
    } else {
      tracingConfig.load(config.getConfigDir() + "/json/trace_config.json");
    }
#endif
    //Sync log files
    sync_log_dir(NeutubeConfig::getInstance().getPath(
                   NeutubeConfig::EConfigItem::LOG_DEST_DIR),
                 NeutubeConfig::getInstance().getPath(
                   NeutubeConfig::EConfigItem::LOG_DIR));
  }

#ifdef _DEBUG_
  config.print();
#endif
}


}


#endif // MAIN_H
