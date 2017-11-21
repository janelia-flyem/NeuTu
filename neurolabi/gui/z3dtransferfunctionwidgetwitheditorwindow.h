#ifndef Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H
#define Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H

#include "zclickablelabel.h"
#include <memory>

class Z3DTransferFunctionEditor;

class Z3DTransferFunctionWidgetWithEditorWindow : public ZClickableTransferFunctionLabel
{
Q_OBJECT
public:
  explicit Z3DTransferFunctionWidgetWithEditorWindow(Z3DTransferFunctionParameter* tf, QWidget* parent = nullptr);

protected:
  void createEditorWindow();

  virtual void labelClicked() override;

private:
  Z3DTransferFunctionParameter* m_transferFunction;
  Z3DTransferFunctionEditor* m_transferFunctionEditor;

  std::unique_ptr<QWidget> m_editorWindow;
};

#endif // Z3DTRANSFERFUNCTIONWIDGETWITHEDITORWINDOW_H
