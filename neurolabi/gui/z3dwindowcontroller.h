#ifndef Z3DWINDOWCONTROLLER_H
#define Z3DWINDOWCONTROLLER_H

class Z3DWindow;

class Z3DWindowController
{
public:
  Z3DWindowController();

  static void SetMeshVisible(Z3DWindow *window, bool on);
  static void ToggleMeshVisible(Z3DWindow *window);
  static void DeselectAllObject(Z3DWindow *window);

};

#endif // Z3DWINDOWCONTROLLER_H
