#ifndef ZEXAMPLEMODULE_H
#define ZEXAMPLEMODULE_H

#include "zsandboxmodule.h"

class ZExampleModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZExampleModule(QObject *parent = 0);

  QAction* getAction() const;

signals:

public slots:

private slots:
  void execute();

private:
  void init();

private:
  QAction *m_action;

};

#endif // ZEXAMPLEMODULE_H
