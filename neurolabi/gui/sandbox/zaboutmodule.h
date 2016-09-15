#ifndef ZABOUTMODULE_H
#define ZABOUTMODULE_H

#include "zsandboxmodule.h"

class ZAboutModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZAboutModule(QObject *parent = 0);

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

#endif // ZABOUTMODULE_H
