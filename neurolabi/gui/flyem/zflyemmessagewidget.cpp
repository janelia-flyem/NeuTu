#include "zflyemmessagewidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QScrollBar>

ZFlyEmMessageWidget::ZFlyEmMessageWidget(QWidget *parent) :
  QWidget(parent)
{
  m_tab = new QTabWidget(this);
  m_oldMessageWidget = new QTextEdit(this);
  m_currentMessageWidget = new QTextEdit(this);

  m_oldMessageWidget->setReadOnly(true);
  m_currentMessageWidget->setReadOnly(true);


  m_tab->addTab(m_currentMessageWidget, "Current");
  m_tab->addTab(m_oldMessageWidget, "Old");
}

void ZFlyEmMessageWidget::dump(const QString &info, bool appending)
{
  if (appending) {
    m_currentMessageWidget->append(info);
    m_currentMessageWidget->verticalScrollBar()->setValue(
          m_currentMessageWidget->verticalScrollBar()->maximum());
  } else {
    m_oldMessageWidget->append(m_currentMessageWidget->toHtml());
    m_oldMessageWidget->clear();
    m_oldMessageWidget->setText(info);
  }
}

void ZFlyEmMessageWidget::dumpError(const QString &info, bool appending)
{
  QString text = "<p><font color=\"#FF0000\">" + info + "</font></p>";

  dump(text, appending);
}
