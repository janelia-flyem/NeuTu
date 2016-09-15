#ifndef ZSANDBOXMODULE_H
#define ZSANDBOXMODULE_H

#include <QObject>

class QMenu;
class QAction;

class ZSandboxModule : public QObject
{
  Q_OBJECT
public:
  explicit ZSandboxModule(QObject *parent = 0);

  virtual QMenu* getMenu() const;
  virtual QAction* getAction() const;

signals:

public slots:

};

#endif // ZSANDBOXMODULE_H
