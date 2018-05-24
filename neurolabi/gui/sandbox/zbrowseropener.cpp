#include <iostream>
#include <QFile>

#include "qaction.h"
#include "zbrowseropener.h"
#include "qdesktopservices.h"
#include "qurl.h"
#include "qstringlist.h"
#include "qprocess.h"

const char* ZBrowserOpener::INTERNAL_BROWSER = "QWebEngine";

ZBrowserOpener::ZBrowserOpener()
{
}

ZBrowserOpener::~ZBrowserOpener()
{
#if defined(_USE_WEBENGINE_)
  if (m_webView != NULL) {
    m_webView->close();
    delete m_webView;
  }
#endif
}

void ZBrowserOpener::open(const QString& url)
{
  open(QUrl(url));
}


void ZBrowserOpener::open(const QUrl& url)
{
  if (m_browerPath == INTERNAL_BROWSER) {
#if defined(_USE_WEBENGINE_)
    if (m_webView == NULL) {
      m_webView = new QWebEngineView;
    }
    qDebug() << "URL: " << url;
    m_webView->setUrl(url);
    m_webView->show();
#endif
  } else if (m_browerPath.isEmpty()) {
    QDesktopServices::openUrl(url);
  } else{
    QProcess process;
    QStringList args;
    args << url.toString();
#ifdef _DEBUG_
    std::cout << "URL: " << args[0].toStdString() << std::endl;
#endif
    process.startDetached(m_browerPath, args);
    //    process.waitForFinished();
    //    system((m_brower_path+" "+url.toString()).toStdString().c_str());
  }
}

bool ZBrowserOpener::findBrowser(QString browser_name)
{
  if (browser_name == INTERNAL_BROWSER) {
    return true;
  }

//  QString find_cmd="find / -executable -type f -or -type l -name \""+browser_name+"\"  -print";
  QString find_cmd = "which " + browser_name;
  QProcess process;
  process.start(find_cmd);
  while(!process.waitForFinished());
  QString output=(process.readAllStandardOutput().toStdString().c_str());
  int index=output.indexOf('\n');
  if(index!=-1){
    m_browerPath=output.left(index);
    return true;
  }
  return false;
}

bool ZBrowserOpener::setChromeBrowser()
{
#if defined(__APPLE__)
  m_browerPath = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
#elif defined(_LINUX_)
  findBrowser("google-chrome");
#endif

  QFile file(m_browerPath);
  if (!file.exists()) {
    m_browerPath.clear();
    return false;
  }

  return true;
}

void ZBrowserOpener::updateChromeBrowser()
{
  if (m_browerPath.isEmpty()) {
    setChromeBrowser();
  }
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
