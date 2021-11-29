#include "neuapp.h"

#include "neutubeconfig.h"
#include "logging/zlog.h"
#include "zdialogfactory.h"

NeuApp::NeuApp(int& argc, char** argv) : QApplication(argc, argv)
{
}


bool NeuApp::notify(QObject* receiver, QEvent* event)
{
  bool done = true;
  try {
    done = QApplication::notify(receiver, event);
  } catch (const std::exception& e) {
    ZERROR(neutu::TOPIC_NULL) << e.what();
    ZDialogFactory::Error(
          "Fatal Error",
          QString("You'd better restart %1 after saving your work in progress.").arg(GET_SOFTWARE_NAME.c_str()),
          e.what(), nullptr);
  }

  return done;
}
