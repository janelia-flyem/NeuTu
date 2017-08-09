#ifndef ZCOLORMAPWIDGETWITHEDITORWINDOW_H
#define ZCOLORMAPWIDGETWITHEDITORWINDOW_H

#include "zclickablelabel.h"

class ZColorMapParameter;

class ZColorMapEditor;

class ZColorMapWidgetWithEditorWindow : public ZClickableColorMapLabel
{
Q_OBJECT
public:
  explicit ZColorMapWidgetWithEditorWindow(ZColorMapParameter* cm, QWidget* parent = nullptr);

protected:
  void createEditorWindow();

  virtual void labelClicked() override;

private:
  ZColorMapParameter* m_colorMap;
  ZColorMapEditor* m_colorMapEditor;

  QWidget* m_editorWindow = nullptr;
};

#endif // ZCOLORMAPWIDGETWITHEDITORWINDOW_H
