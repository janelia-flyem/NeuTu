#include "zmainwindowcontroller.h"
#include "zjsonobject.h"
#include "mainwindow.h"
#include "zflyemproofmvc.h"
#include "zproofreadwindow.h"
#include "dvid/zdvidurl.h"

ZMainWindowController::ZMainWindowController()
{
}

void ZMainWindowController::StartTestTask(ZProofreadWindow *window)
{
  if (window != NULL) {
    window->getMainMvc()->startTestTask(ZDvidUrl::GetTaskKey());
  }
}

void ZMainWindowController::StartTestTask(MainWindow *mainWin)
{
  if (mainWin != NULL) {
    StartTestTask(mainWin->startProofread());
  }
}

/*
void ZMainWindowController::StartTestTask(const std::string &taskKey)
{
  //todo
  MainWindow *mainWin = ZGlobal::getMainWindow<MainWindow>();
  if (mainWin != NULL) {
    ZProofreadWindow *window = mainWin->startProofread();
    StartTestTask(window);
  }
}
*/
