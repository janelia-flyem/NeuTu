#ifndef ZBROWSEROPENER_H
#define ZBROWSEROPENER_H
#include "zsandboxmodule.h"

class QUrl;
class QString;

class ZBrowserOpener
{
public:
  ZBrowserOpener();

public:
  static void open(const QString& url);
  static void open(const QUrl& url);
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
