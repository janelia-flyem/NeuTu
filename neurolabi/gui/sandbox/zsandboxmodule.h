#ifndef ZSANDBOXMODULE_H
#define ZSANDBOXMODULE_H

#include <QObject>

class QMenu;
class QAction;
class MainWindow;
class ZStackDoc;

class ZSandboxModule : public QObject
{
  Q_OBJECT
public:
  explicit ZSandboxModule(QObject *parent = 0);

  QMenu* getMenu() const;
  QAction* getAction() const;

//  virtual void initialize(MainWindow *mainWindow);

signals:
  void docGenerated(ZStackDoc*);

public slots:


protected:
  void setMenu(QMenu *menu);
  void setAction(QAction *action);

private:
  void init();

protected:
  QAction *m_action;
  QMenu *m_menu;
};

#endif // ZSANDBOXMODULE_H
