#ifndef ZSUBTRACTSWCSDIALGO_H
#define ZSUBTRACTSWCSDIALGO_H

#include <QDialog>

class ZSelectFileWidget;
class QDialogButtonBox;

class ZSubtractSWCsDialog : public QDialog
{
  Q_OBJECT
public:
  ZSubtractSWCsDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

private:
  void init();

private slots:
  void subtractSWCs();

private:
  ZSelectFileWidget *m_inputSWCWidget;
  ZSelectFileWidget *m_subtractSWCsWidget;
  ZSelectFileWidget *m_outputSWCWidget;

  QPushButton *m_runButton;
  QPushButton *m_exitButton;
  QDialogButtonBox *m_buttonBox;
};

#endif // ZSUBTRACTSWCSDIALGO_H
