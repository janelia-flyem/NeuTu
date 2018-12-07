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

#include "main.h"

#include "mainwindow.h"
#include "neu3window.h"

#include "ztest.h"

#include "tz_utilities.h"

#include "core/utilities.h"
#include "sandbox/zsandboxproject.h"
#include "sandbox/zsandbox.h"
#include "flyem/zmainwindowcontroller.h"


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
    std::cout << neutube::GetVersionString() << std::endl;

    return 0;
  }

  NeutubeConfig::getInstance().init(mainConfig.userName);

  if (mainConfig.runCommandLine) {
    return run_command_line(argc, argv);
  }

  if (mainConfig.isGuiEnabled()) {
    init_gui();
  }

  // call first otherwise it will cause runtime warning:
  //   Please instantiate the QApplication object first
  QApplication app(argc, argv, mainConfig.isGuiEnabled());

  neutube::RegisterMetaType();

  configure(mainConfig);

  // init the logging mechanism
  init_log();

//  RECORD_INFORMATION("************* Start ******************");

  LINFO() << "Config path: " << mainConfig.configPath;

  if (mainConfig.isGuiEnabled()) {
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

    if (!mainConfig.runCommandLine) {
      //Sync log files
      sync_log_dir(NeutubeConfig::getInstance().getPath(
                     NeutubeConfig::EConfigItem::LOG_DIR),
                 NeutubeConfig::getInstance().getPath(
                     NeutubeConfig::EConfigItem::LOG_DEST_DIR));
    }

    return result;
  } else {
    /********* for debugging *************/

#ifndef QT_NO_DEBUG
    if (mainConfig.unitTest) {
      ZTest::RunUnitTest(argc, argv);
    }
#else
    if (mainConfig.unitTest) {
      std::cout << "No unit test in the release version." << std::endl;
    }
#endif
    if (!mainConfig.unitTest) {
      std::cout << "Running test function" << std::endl;
      ZTest::test(NULL);
    }

    return 1;
  }
}
#endif

