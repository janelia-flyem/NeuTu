#include <iostream>
#include "qaction.h"
#include "zbrowseropener.h"
#include "qdesktopservices.h"
#include "qurl.h"
#include "qstringlist.h"
#include "qprocess.h"

ZBrowserOpener::ZBrowserOpener()
{

}

void ZBrowserOpener::open(const QString& url)
{
  open(QUrl(url));
}


void ZBrowserOpener::open(const QUrl& url)
{
  if(m_brower_path==""){
    QDesktopServices::openUrl(url);
  }
  else{
    system((m_brower_path+" "+url.toString()).toStdString().c_str());
  }
}

bool ZBrowserOpener::findBrowser(QString browser_name)
{
  QString find_cmd="find / -executable -type f -or -type l -name \""+browser_name+"\"  -print";
  QProcess process;
  process.start(find_cmd);
  while(!process.waitForFinished());
  QString output=(process.readAllStandardOutput().toStdString().c_str());
  int index=output.indexOf('\n');
  if(index!=-1){
    m_brower_path=output.left(index);
    return true;
  }
  return false;
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
  ZBrowserOpener browser;
  browser.findBrowser("google-chrome");
  browser.open("http://www.baidu.com");
}
