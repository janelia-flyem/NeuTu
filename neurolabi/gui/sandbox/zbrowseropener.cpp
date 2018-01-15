#include "qaction.h"
#include "zbrowseropener.h"
#include "qdesktopservices.h"
#include "qurl.h"

ZBrowserOpener::ZBrowserOpener()
{

}

void ZBrowserOpener::open(const QString& url)
{
  open(QUrl(url));
}


void ZBrowserOpener::open(const QUrl& url)
{
  QDesktopServices::openUrl(url);
}


ZBrowserOpenerModule::ZBrowserOpenerModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZBrowserOpenerModule::init()
{
  QAction *action = new QAction("open url", this);
  connect(action, SIGNAL(triggered()), this, SLOT(execute()));

  setAction(action);
}

void ZBrowserOpenerModule::execute()
{
  ZBrowserOpener::open("http://www.baidu.com");
}
