#ifndef ZBROWSEROPENER_H
#define ZBROWSEROPENER_H
#include "zsandboxmodule.h"

class QUrl;
class QString;

class ZBrowserOpener
{
public:
  ZBrowserOpener();
  ZBrowserOpener(QString browser,bool path=false){
    if(path){
      setBrowserPath(browser);
    }
    else{
      findBrowser(browser);
    }
  }
public:
  bool findBrowser(QString brower_name);
  void setBrowserPath(QString brower_path){
    m_browerPath=brower_path;
  }
  void open(const QString& url);
  void open(const QUrl& url);

  bool setChromeBrowser();
  const QString& getBrowserPath() const {
    return m_browerPath;
  }

  /*!
   * \brief Try to set the browser to chrome if it is not set.
   */
  void updateChromeBrowser();

private:
  QString m_browerPath;
};


class ZBrowserOpenerModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZBrowserOpenerModule(QObject *parent = 0);

signals:

public slots:

private slots:
  void execute();

private:
  void init();

};

#endif // ZBROWSEROPENER_H
