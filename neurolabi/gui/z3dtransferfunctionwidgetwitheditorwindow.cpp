#include "z3dtransferfunctionwidgetwitheditorwindow.h"

#include "z3dtransferfunction.h"
#include "z3dtransferfunctioneditor.h"
#include <QApplication>

Z3DTransferFunctionWidgetWithEditorWindow::Z3DTransferFunctionWidgetWithEditorWindow(Z3DTransferFunctionParameter* tf,
                                                                                     QWidget* parent)
  : ZClickableTransferFunctionLabel(tf, parent)
  , m_transferFunction(tf)
{
}

void Z3DTransferFunctionWidgetWithEditorWindow::createEditorWindow()
{
  if (m_editorWindow) {
    m_editorWindow->showNormal();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();
  } else {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_editorWindow.reset(new Z3DTransferFunctionEditor(m_transferFunction));
    m_editorWindow->showNormal();
    m_editorWindow->raise();
    m_editorWindow->activateWindow();

    QApplication::restoreOverrideCursor();
  }
}

void Z3DTransferFunctionWidgetWithEditorWindow::labelClicked()
{
  createEditorWindow();
}
