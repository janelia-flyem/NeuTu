#include <iostream>

#ifdef _CLI_VERSION
#include "zcommandline.h"

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

#include <cstring>

#include <QProcess>
#include <QDir>
#include <QSplashScreen>
#include <QElapsedTimer>

#include "main.h"

#include "mainwindow.h"
#include "neu3window.h"

#include "ztest.h"
#include "qfonticon.h"
#include "common/utilities.h"
#include "sandbox/zsandboxproject.h"
#include "sandbox/zsandbox.h"
#include "flyem/zmainwindowcontroller.h"
#include "zglobal.h"
#include "logging/zlog.h"

namespace {
std::string get_machine_info()
{
  return GET_SOFTWARE_NAME + " " + neutu::GetVersionString() +
      " @{" + QSysInfo::prettyProductName().toStdString() + "}";
}
}

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

  MainConfig mainConfig = get_program_config(argc, argv);

  if (mainConfig.showingVersion) {
    std::cout << argv[0] << std::endl;
    std::cout << neutu::GetVersionString() << std::endl;

    return 0;
  }

  NeutubeConfig::getInstance().init(mainConfig.userName);

  if (mainConfig.runCommandLine) {
    NeutubeConfig::getInstance().setCliSoftwareName("cli");
    ZGlobal::InitKafkaTracer();
    KLog::SetOperationName("cli");
    LKLOG << ZLog::Info()
           << ZLog::Description("BEGIN " + GET_SOFTWARE_NAME);
    return run_command_line(argc, argv);
  }

  if (mainConfig.isGuiEnabled()) {
    init_gui();
  }

  // call first otherwise it will cause runtime warning:
  //   Please instantiate the QApplication object first
  QApplication app(argc, argv, mainConfig.isGuiEnabled());

  neutu::RegisterMetaType();

  configure(mainConfig);

  // init the logging mechanism
  init_log();

//  RECORD_INFORMATION("************* Start ******************");

  LINFO() << "Config path: " << mainConfig.configPath;

  if (mainConfig.isGuiEnabled()) {
    QElapsedTimer appTimer;
    appTimer.start();

    QFontIcon::addFont(":/fontawesome.ttf");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#if defined(__APPLE__)
    QPixmap pixmap(QString::fromStdString(GET_CONFIG_DIR + "/splash.png"));
    QSplashScreen splash(pixmap);
    splash.show();
    splash.showMessage("Preparing user environment ...");
#endif

    NeutubeConfig::UpdateUserInfo();

#if defined(__APPLE__)
    splash.showMessage("Initializing logging ...");
#endif

    ZGlobal::InitKafkaTracer();

    uint64_t timestamp = neutu::GetTimestamp();
    KLog() << ZLog::Info() //<< ZLog::Time(timestamp)
           << ZLog::Description("BEGIN " + get_machine_info())
           << ZLog::Diagnostic("config:" + mainConfig.configPath.toStdString());
    LINFO() << "Start " + get_machine_info();
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

#if defined(__APPLE__)
    splash.showMessage("Preparing work directories ...");
#endif

    MainWindow::createWorkDir();
    NeutubeConfig::UpdateAutoSaveDir();

#if (defined __APPLE__) && !(defined _QT5_)
    app.setGraphicsSystem("raster");
#endif

    ZTest::getInstance().setCommandLineArg(argc, argv);

#ifdef _NEU3_
    Neu3Window *mainWin = new Neu3Window();

#if defined(__APPLE__)
    splash.close();
#endif

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

    if (!mainConfig.fileList.isEmpty()) {
      mainWin->showStackFrame(mainConfig.fileList, true);
    }

    if (argc > 1) {
      mainWin->processArgument(argv[1]);
    } /*else {
      mainWin->processArgument(QString("test %1: %2").arg(argc).arg(argv[0]));
    }*/

    ZSandbox::SetMainWindow(mainWin);
    ZSandboxProject::InitSandbox();
#  if defined(__APPLE__)
    splash.finish(mainWin);
#  endif
#endif

    int result = 1;

    if (mainWin != NULL) {
      if (mainConfig.autoTestingTask) {
#if defined(_FLYEM_) && !defined(_NEU3_)
#  if defined(_DEBUG_)
        ZMainWindowController::StartTestTask(mainWin);
#  else
        ZMainWindowController::StartTestTask(mainWin->startProofread());
#  endif
#endif
      } else {
#if !defined(_DEBUG_) && !defined(_NEU3_)
        mainWin->startProofread();
#endif
      }

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

    if (!mainConfig.runCommandLine) {
      //Sync log files
      sync_log_dir(NeutubeConfig::getInstance().getPath(
                     NeutubeConfig::EConfigItem::LOG_DIR),
                 NeutubeConfig::getInstance().getPath(
                     NeutubeConfig::EConfigItem::LOG_DEST_DIR));
    }

    KLog() << ZLog::Info()
           << ZLog::Description("END " + get_machine_info())
           << ZLog::Tag("start_time", timestamp)
           << ZLog::Duration(appTimer.elapsed());
    LINFO() << "Exit " + get_machine_info();

    return result;
  } else {
    /********* for debugging *************/

//#ifndef QT_NO_DEBUG
    if (mainConfig.unitTest) {
//      ZUnitTest(argc, argv).run();
      return ZTest::RunUnitTest(argc, argv);
    } else {
      std::cout << "Running test function" << std::endl;
      ZTest::test(NULL);

      return 0;
    }
//#else
//    if (mainConfig.unitTest) {
//      std::cout << "No unit test in the release version." << std::endl;
//    }
//#endif
//    if (!mainConfig.unitTest) {
//      std::cout << "Running test function" << std::endl;
//      ZTest::test(NULL);
//    }

  }
}
#endif

