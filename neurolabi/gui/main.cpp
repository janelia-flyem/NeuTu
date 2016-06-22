#include <iostream>
#include <cstring>
#include <QApplication>
#include <QDir>
#include "mainwindow.h"
#include "QsLog/QsLog.h"
#include "QsLog/QsLogDest.h"
#include "zcommandline.h"
#include "zerror.h"
#include "z3dapplication.h"
#include "zneurontracer.h"

#include "ztest.h"

#include "tz_utilities.h"
#include "neutubeconfig.h"
#include "zneurontracerconfig.h"

#ifdef _QT5_
#include <QSurfaceFormat>

#include <QStack>
#include <QPointer>
// thanks to Daniel Price for this workaround
struct MacEventFilter : public QObject
{
  QStack<QPointer<QWidget> > m_activationstack; // stack of widgets to re-active on dialog close.

  explicit MacEventFilter(QObject *parent = NULL)
    : QObject(parent)
  {}

  virtual bool eventFilter(QObject *anObject, QEvent *anEvent)
  {
    switch (anEvent->type()) {
    case QEvent::Show: {
      if ((anObject->inherits("QDialog") || anObject->inherits("QDockWidget")) && qApp->activeWindow()) {
        // Workaround for Qt bug where opened QDialogs do not re-activate previous window
        // when accepted or rejected. We cannot rely on the parent pointers so push the previous
        // active window onto a stack before the dialog is shown.
        // We have to use a stack in case a dialog opens another dialog.
        // NOTE: It's important to use QPointers so that any widgets deleted by Qt do not lead to
        // hanging pointers in the stack.
        m_activationstack.push(qApp->activeWindow());
      }
      break;
    }
    case QEvent::Hide: {
      if ((anObject->inherits("QDialog") || anObject->inherits("QDockWidget")) && !m_activationstack.isEmpty()) {
        QPointer<QWidget> widget = m_activationstack.pop();
        if (widget) {
          // Re-acivate widgets in the order as dialogs are closed. See Show case above.
          widget->activateWindow();
          widget->raise();
        }
      }
      break;
    }
    default:
      break;
    }

    return QObject::eventFilter(anObject, anEvent);
  }
};

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

int main(int argc, char *argv[])
{
#ifdef _QT5_
  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
#endif
  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);

#ifdef _QT5_
  QSurfaceFormat format;
  //format.setStereo(true);
  QSurfaceFormat::setDefaultFormat(format);
#endif

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

  QStringList fileList;

  bool guiEnabled = true;

  if (argc > 1) {
    if (strcmp(argv[1], "d") == 0) {
      debugging = true;
    }

    if (strcmp(argv[1], "--command") == 0) {
      runCommandLine = true;
    }

    if (runCommandLine) {
      ZCommandLine cmd;
      return cmd.run(argc, argv);
    }


#ifndef QT_NO_DEBUG
    if (strcmp(argv[1], "u") == 0 || QString(argv[1]).startsWith("--gtest")) {
      unitTest = true;
      debugging = true;
    }

    if (strcmp(argv[1], "--load") == 0) {
      for (int i = 2; i < argc; ++i) {
        fileList << argv[i];
      }
    }
#endif
  }
  if (debugging || runCommandLine) {
    guiEnabled = false;
  }


  // call first otherwise it will cause runtime warning: Please instantiate the QApplication object first
  QApplication app(argc, argv, guiEnabled);

  //load config
  NeutubeConfig &config = NeutubeConfig::getInstance();
  std::cout << QApplication::applicationDirPath().toStdString() << std::endl;
  config.setApplicationDir(QApplication::applicationDirPath().toStdString());
  if (config.load(config.getConfigPath()) == false) {
    std::cout << "Unable to load configuration: "
              << config.getConfigPath() << std::endl;
  }

  if (!runCommandLine) { //Command line mode takes care of configuration independently
    ZNeuronTracerConfig &tracingConfig = ZNeuronTracerConfig::getInstance();
    if (GET_APPLICATION_NAME == "Biocytin") {
      tracingConfig.load(
            config.getApplicatinDir() + "/json/trace_config_biocytin.json");
    } else {
      tracingConfig.load(config.getApplicatinDir() + "/json/trace_config.json");
    }
  }

#ifdef _DEBUG_
  config.print();
#endif

  // init the logging mechanism
  QsLogging::Logger& logger = QsLogging::Logger::instance();
  const QString sLogPath(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::LOG_FILE).c_str());
  QsLogging::DestinationPtr fileDestination(
        QsLogging::DestinationFactory::MakeFileDestination(
          sLogPath, QsLogging::EnableLogRotation,
          QsLogging::MaxSizeBytes(5e7), QsLogging::MaxOldLogCount(50)));
  QsLogging::DestinationPtr debugDestination(
        QsLogging::DestinationFactory::MakeDebugOutputDestination());
  logger.addDestination(debugDestination);
  logger.addDestination(fileDestination);
#if defined _DEBUG_
  logger.setLoggingLevel(QsLogging::DebugLevel);
#else
  logger.setLoggingLevel(QsLogging::InfoLevel);
#endif

  RECORD_INFORMATION("************* Start ******************");

  if (guiEnabled) {
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

#if (defined __APPLE__) && !(defined _QT5_)
    app.setGraphicsSystem("raster");
#endif

#ifdef _QT5_
    qApp->installEventFilter(new MacEventFilter(qApp));
#endif

    LINFO() << "Start " + GET_SOFTWARE_NAME + " - " + GET_APPLICATION_NAME;

    // init 3D
    //std::cout << "Initializing 3D ..." << std::endl;
    RECORD_INFORMATION("Initializing 3D ...");
    Z3DApplication z3dApp(QCoreApplication::applicationDirPath());
    z3dApp.initialize();

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

    int result =  app.exec();

    delete mainWin;
    z3dApp.deinitializeGL();
    z3dApp.deinitialize();

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
    /*
    std::cout << "Debugging ..." << std::endl;
    ZCurve curve;
    double *array = new double[10];
    for (int i = 0; i < 10; i++) {
      array[i] = -i * i;
    }
    curve.loadArray(array, 10);
    std::cout << curve.minY() << std::endl;
    */
    if (unitTest) {
      ZTest::runUnitTest(argc, argv);
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
