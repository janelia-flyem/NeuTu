#ifndef ZSANDBOX_H
#define ZSANDBOX_H

class MainWindow;
class ZStackFrame;
class ZStackDoc;
class ZSandboxModule;

class ZSandbox
{
public:
  ZSandbox();

  static ZSandbox& GetInstance() {
    static ZSandbox sandbox;

    return sandbox;
  }


  static MainWindow* GetMainWindow();
  static ZStackFrame* GetCurrentFrame();
  static ZStackDoc* GetCurrentDoc();

  static void SetMainWindow(MainWindow *win);

  static void RegisterModule(ZSandboxModule *module);

private:
  void setMainWindow(MainWindow *win);

  MainWindow* getMainWindow() const;
  ZStackFrame* getCurrentFrame() const;
  ZStackDoc* getCurrentDoc() const;
  void registerModule(ZSandboxModule *module);

private:
  void init();

private:
  MainWindow *m_mainWin;

};

#endif // ZSANDBOX_H
