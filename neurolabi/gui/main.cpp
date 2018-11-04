#include <iostream>
#include <cstring>
#include <QApplication>
#include <QProcess>
#include <QDir>

#ifdef _QT5_
#include <QSurfaceFormat>
#endif

#include "mainwindow.h"
#include "neu3window.h"
#include "zqslog.h"
#include "QsLog/QsLogDest.h"
#include "zcommandline.h"
#include "zerror.h"
#include "zneurontracer.h"
#include "zapplication.h"

#include "ztest.h"

#include "tz_utilities.h"
#include "neutubeconfig.h"
#include "zneurontracerconfig.h"
#include "core/utilities.h"
#include "sandbox/zsandboxproject.h"
#include "sandbox/zsandbox.h"
#include "flyem/zmainwindowcontroller.h"
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

namespace neutube {
static std::string UserName;
}

static void syncLogDir(const std::string &srcDir, const std::string &destDir)
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

namespace {

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

} //namespace

static void LoadFlyEmConfig(
    const QString &configPath, NeutubeConfig &/*config*/, bool usingConfig)
{
#ifdef _FLYEM_
  ZJsonObject configObj;
  if (!configPath.isEmpty()) {
    configObj.load(configPath.toStdString());
  }

  GET_FLYEM_CONFIG.useDefaultConfig(NeutubeConfig::UsingDefaultFlyemConfig());
  GET_FLYEM_CONFIG.useDefaultNeuTuServer(NeutubeConfig::UsingDefaultNeuTuServer());
  GET_FLYEM_CONFIG.useDefaultTaskServer(NeutubeConfig::UsingDefaultTaskServer());

//  QDir appDir((GET_APPLICATION_DIR).c_str());
//  QFileInfo configFileInfo = QFileInfo(appDir, "local_flyem_config.json");
//  if (!configFileInfo.exists()) {
//    configFileInfo.setFile(appDir, "flyem_config.json");
//  }

//  QString defaultFlyemConfigPath = configFileInfo.absoluteFilePath();
//  GET_FLYEM_CONFIG.setDefaultConfigPath(defaultFlyemConfigPath.toStdString());

//  QString flyemConfigPath = NeutubeConfig::GetFlyEmConfigPath();
//  if (flyemConfigPath.isEmpty()) {
//    QFileInfo configFileInfo(configPath);

//    flyemConfigPath = ZJsonParser::stringValue(configObj["flyem"]);
//    if (flyemConfigPath.isEmpty()) {
//      flyemConfigPath = defaultFlyemConfigPath;
//    } else {
//      QFileInfo flyemConfigFileInfo(flyemConfigPath);
//      if (!flyemConfigFileInfo.isAbsolute()) {
//        flyemConfigPath =
//            configFileInfo.absoluteDir().absoluteFilePath(flyemConfigPath);
//      }
//    }
//  }

  SetFlyEmConfigpath(configPath, configObj);

  GET_FLYEM_CONFIG.loadConfig();
  GET_FLYEM_CONFIG.loadUserSettings();

  //Settings provided by a more general source
  if (usingConfig) {
#ifdef _DEBUG_2
    std::cout << "NeuTu server: " << config.GetNeuTuServer().toStdString() << std::endl;
#endif

    if (GET_FLYEM_CONFIG.hasDefaultNeuTuServer() == false) {
      QString neutuServer = ZJsonParser::stringValue(configObj["neutu_server"]);
      if (!neutuServer.isEmpty()) {
        GET_FLYEM_CONFIG.setCustomNeuTuServer(neutuServer.toStdString());
        //        GET_FLYEM_CONFIG.setDefaultNeuTuServer(neutuServer.toStdString());
      }
    }

#ifdef _DEBUG_2
    GET_FLYEM_CONFIG.setServer("neutuse:http://127.0.0.1:5000");
#endif

    if (GET_FLYEM_CONFIG.hasDefaultTaskServer() == false) {
      QString taskServer = ZJsonParser::stringValue(configObj["task_server"]);
      if (!taskServer.isEmpty()) {
        GET_FLYEM_CONFIG.setCustomTaskServer(taskServer.toStdString());
      }
    }
//      GET_FLYEM_CONFIG.setDefaultTaskServer(taskServer.toStdString());
  }
#endif
}

static void InitLog()
{
  // init the logging mechanism
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  const QString sLogPath(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_FILE).c_str());
  const QString traceLogPath(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_TRACE).c_str());

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

#ifdef _CLI_VERSION
int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "--command") == 0)
  {
    return ZCommandLine().run(argc,argv);
  }
  else
  {
    std::cout<<"This is CLI version of neutu,please use --command option."<<std::endl;
    return 1;
  }
}
#else
int main(int argc, char *argv[])
{
#if 0 //Disable redirect for explicit logging
#ifndef _FLYEM_
#ifdef _QT5_
  qInstallMessageHandler(myMessageOutput);
#else
  qInstallMsgHandler(myMessageOutput);
#endif
#endif
#endif

  bool debugging = false;
  bool unitTest = false;
  bool runCommandLine = false;

  bool guiEnabled = true;
  bool advanced = false;

  QString configPath;
  QStringList fileList;

  std::string userName;
  if (argc > 1) {
    if (QString(argv[1]).startsWith("user:")) {
      userName = std::string(argv[1]).substr(5);
    }
  }
  if (userName.empty()) {
    userName = qgetenv("USER").toStdString();
  }
  NeutubeConfig::getInstance().init(userName);

  if (argc > 1) {
    if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
      std::cout << argv[0] << std::endl;
      std::cout << neutube::GetVersionString() << std::endl;

      return 0;
    }

    if (strcmp(argv[1], "d") == 0) {
      debugging = true;
    }

    if (strcmp(argv[1], "a") == 0) {
      advanced = true;
    }

    if (strcmp(argv[1], "--command") == 0) {
      runCommandLine = true;
    }

    if (runCommandLine) {
#if defined(_FLYEM_)
      NeutubeConfig &config = NeutubeConfig::getInstance();
      QFileInfo fileInfo(argv[0]);
      std::string appDir = fileInfo.absoluteDir().absolutePath().toStdString();
      config.setApplicationDir(appDir);
      LoadFlyEmConfig("", config, false);
#endif

      InitLog();

      ZCommandLine cmd;
      return cmd.run(argc, argv);
    }

    if (strcmp(argv[1], "u") == 0 || QString(argv[1]).startsWith("--gtest")) {
      unitTest = true;
      debugging = true;
    }

    if (strcmp(argv[1], "--load") == 0) {
      for (int i = 2; i < argc; ++i) {
        fileList << argv[i];
      }
    }

    if (QString(argv[1]).endsWith(".json")) {
      configPath = argv[1];
    }
  }
  if (debugging || runCommandLine) {
    guiEnabled = false;
  }

  if (guiEnabled) {
    GET_FLYEM_CONFIG.activateNeuTuServer();

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

  // call first otherwise it will cause runtime warning: Please instantiate the QApplication object first
  QApplication app(argc, argv, guiEnabled);

  neutube::RegisterMetaType();

  //load config
  NeutubeConfig &config = NeutubeConfig::getInstance();
  config.setAdvancedMode(advanced);

  std::cout << QApplication::applicationDirPath().toStdString() << std::endl;
  config.setApplicationDir(QApplication::applicationDirPath().toStdString());

  if (config.load(config.getConfigPath()) == false) {
    std::cout << "Unable to load configuration: "
              << config.getConfigPath() << std::endl;
  }

  if (configPath.isEmpty()) {
    configPath =
        QFileInfo(QDir((GET_APPLICATION_DIR + "/json").c_str()), "config.json").
        absoluteFilePath();
  }

#ifdef _FLYEM_
  LoadFlyEmConfig(configPath, config, true);

  ZGlobalDvidRepo::GetInstance().init();
#endif

  if (!runCommandLine) { //Command line mode takes care of configuration independently
#if !defined(_FLYEM_)
    ZNeuronTracerConfig &tracingConfig = ZNeuronTracerConfig::getInstance();
    tracingConfig.load(config.getApplicatinDir() + "/json/trace_config.json");

    if (GET_APPLICATION_NAME == "Biocytin") {
      tracingConfig.load(
            config.getApplicatinDir() + "/json/trace_config_biocytin.json");
    } else {
      tracingConfig.load(config.getApplicatinDir() + "/json/trace_config.json");
    }
#endif
    //Sync log files
    syncLogDir(NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_DEST_DIR),
               NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_DIR));
  }

#ifdef _DEBUG_
  config.print();
#endif

  // init the logging mechanism
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  const QString sLogPath(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_FILE).c_str());
  const QString traceLogPath(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_TRACE).c_str());

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

//  RECORD_INFORMATION("************* Start ******************");

  LINFO() << "Config path: " << configPath;

  if (guiEnabled) {
    LINFO() << "Start " + GET_SOFTWARE_NAME + " - " + GET_APPLICATION_NAME
            + " " + neutube::GetVersionString();
#if defined __APPLE__        //use macdeployqt
#else
#if defined(QT_NO_DEBUG)
    QDir dir(QApplication::applicationDirPath());
    dir.cd("plugins");
    QApplication::addLibraryPath(dir.absolutePath());  // for windows version
    dir.cdUp();
    dir.cdUp();
    dir.cd("plugins");
    QApplication::addLibraryPath(dir.absolutePath());
    dir.cdUp();
    dir.cd("lib");
    QApplication::addLibraryPath(dir.absolutePath());
#endif
#endif

    MainWindow::createWorkDir();
    NeutubeConfig::UpdateAutoSaveDir();

#if (defined __APPLE__) && !(defined _QT5_)
    app.setGraphicsSystem("raster");
#endif

    ZTest::getInstance().setCommandLineArg(argc, argv);

    // init 3D
    //std::cout << "Initializing 3D ..." << std::endl;
    RECORD_INFORMATION("Initializing 3D ...");
#ifdef _NEU3_
    Neu3Window *mainWin = new Neu3Window();

    if (!mainWin->loadDvidTarget()) {
      mainWin->close();
//      delete mainWin;
      mainWin = NULL;
    }
#else
    MainWindow *mainWin = new MainWindow();
    mainWin->configure();
    mainWin->show();
    mainWin->raise();
    mainWin->initOpenglContext();

    if (!fileList.isEmpty()) {
      mainWin->showStackFrame(fileList, true);
    }

    if (argc > 1) {
      mainWin->processArgument(argv[1]);
    } /*else {
      mainWin->processArgument(QString("test %1: %2").arg(argc).arg(argv[0]));
    }*/

    ZSandbox::SetMainWindow(mainWin);
    ZSandboxProject::InitSandbox();
#endif

    int result = 1;

    if (mainWin != NULL) {
#if defined(_FLYEM_) && !defined(_NEU3_)
#  if defined(_DEBUG_)
      ZMainWindowController::StartTestTask(mainWin);
#  else
      ZMainWindowController::StartTestTask(mainWin->startProofread());
#  endif
#endif

#if defined(_NEU3_2)
      mainWin->show();
      mainWin->initialize();
      mainWin->raise();
      mainWin->showMaximized();
#endif

      try {
        result = app.exec();
      } catch (std::exception &e) {
        LERROR() << "Crashed by exception:" << e.what();
      }

      delete mainWin;
    }

    if (!runCommandLine) {
      //Sync log files
      syncLogDir(NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_DIR),
                 NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_DEST_DIR));
    }

    return result;
  } else {
    /*
    if (runCommandLine) {
      ZCommandLine cmd;
      return cmd.run(argc, argv);
    }
    */

    /********* for debugging *************/

#ifndef QT_NO_DEBUG
    if (unitTest) {
      ZTest::RunUnitTest(argc, argv);
    }
#else
    if (unitTest) {
      std::cout << "No unit test in the release version." << std::endl;
    }
#endif
    if (!unitTest) {
      std::cout << "Running test function" << std::endl;
      ZTest::test(NULL);
    }

    return 1;
  }
}
#endif
