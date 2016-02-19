#ifndef ZFLYEMMESSAGEWIDGET_H
#define ZFLYEMMESSAGEWIDGET_H

#include <QWidget>
#include <QTabWidget>

#include "neutube.h"

class QTextEdit;

class ZFlyEmMessageWidget : public QTabWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmMessageWidget(QWidget *parent = 0);
  QTabWidget* getTab();

signals:

public slots:
  void dump(const QString &info, bool appending);
  void dumpError(const QString &info, bool appending);
  void dump(const QStringList &info);
  void dumpError(const QStringList &info);
  void dump(const QString &info, NeuTube::EMessageType type,
            bool appending);
  void dump(const QStringList &info, NeuTube::EMessageType type,
            bool appending);

private:
//  QTabWidget *m_tab;
  QTextEdit *m_currentMessageWidget;
  QTextEdit *m_oldMessageWidget;
};

#endif // ZFLYEMMESSAGEWIDGET_H
