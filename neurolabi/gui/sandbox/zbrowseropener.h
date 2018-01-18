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
    m_brower_path=brower_path;
  }
  void open(const QString& url);
  void open(const QUrl& url);
private:
  QString m_brower_path;
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
