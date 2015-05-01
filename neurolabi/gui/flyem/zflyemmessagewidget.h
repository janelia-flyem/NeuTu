#ifndef ZFLYEMMESSAGEWIDGET_H
#define ZFLYEMMESSAGEWIDGET_H

#include <QWidget>

class QTabWidget;
class QTextEdit;

class ZFlyEmMessageWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmMessageWidget(QWidget *parent = 0);

signals:

public slots:
  void dump(const QString &info, bool appending);
  void dumpError(const QString &info, bool appending);

private:
  QTabWidget *m_tab;
  QTextEdit *m_currentMessageWidget;
  QTextEdit *m_oldMessageWidget;
};

#endif // ZFLYEMMESSAGEWIDGET_H
