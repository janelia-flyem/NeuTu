#include "zapplication.h"
#include "QsLog.h"

ZApplication::ZApplication(int argc, char **argv, bool GUIenabled) :
  QApplication(argc, argv, GUIenabled)
{
}

bool ZApplication::notify(QObject *receiver, QEvent *event)
{
  try {
    return QApplication::notify(receiver, event);
  } catch (std::exception& e) {
    LERROR() << "## !!FATAL!! Exception thrown: " << e.what();
  }
  return false;
}
