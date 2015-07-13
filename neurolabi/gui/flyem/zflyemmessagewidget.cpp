#include "zflyemmessagewidget.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QScrollBar>
#include <QPushButton>
#include <QDebug>

#include "zwidgetmessage.h"

ZFlyEmMessageWidget::ZFlyEmMessageWidget(QWidget *parent) :
  QTabWidget(parent)
{
//  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
//  getTab() = new QTabWidget(this);
  m_oldMessageWidget = new QTextEdit(this);
  m_currentMessageWidget = new QTextEdit(this);

  m_currentMessageWidget->setStyleSheet("background-color:white;");
  m_oldMessageWidget->setStyleSheet("background-color:white;");

  m_oldMessageWidget->setReadOnly(true);
  m_currentMessageWidget->setReadOnly(true);

  getTab()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  getTab()->setTabPosition(QTabWidget::West);
  getTab()->addTab(m_currentMessageWidget, "Current");
  getTab()->addTab(m_oldMessageWidget, "Old");
//  getTab()->addTab(new QPushButton("test", this), "test");
}

void ZFlyEmMessageWidget::dump(const QString &info, bool appending)
{
  if (appending) {
//    m_currentMessageWidget->insertHtml(info);
    m_currentMessageWidget->setTextColor(QColor("black"));
    m_currentMessageWidget->append(info);
    m_currentMessageWidget->verticalScrollBar()->setValue(
          m_currentMessageWidget->verticalScrollBar()->maximum());
  } else {
    m_oldMessageWidget->append(m_currentMessageWidget->toHtml());
    m_currentMessageWidget->clear();
    m_currentMessageWidget->setText(info);
  }

//  qDebug() << m_currentMessageWidget->toHtml();
}

void ZFlyEmMessageWidget::dump(const QStringList &info)
{
  if (!info.isEmpty()) {
    dump(info[0], false);
    for (int i = 1; i < info.size(); ++i) {
      dump(info[1], true);
    }
  }
}

void ZFlyEmMessageWidget::dumpError(const QStringList &info)
{
  if (!info.isEmpty()) {
    dumpError(info[0], false);
    for (int i = 1; i < info.size(); ++i) {
      dumpError(info[1], true);
    }
  }
}

void ZFlyEmMessageWidget::dumpError(const QString &info, bool appending)
{
  dump(info, NeuTube::MSG_ERROR, appending);
}

void ZFlyEmMessageWidget::dump(
    const QString &info, NeuTube::EMessageType type, bool appending)
{
  dump(ZWidgetMessage::ToHtmlString(info, type), appending);
}

void ZFlyEmMessageWidget::dump(
    const QStringList &info, NeuTube::EMessageType type, bool appending)
{
  if (!info.isEmpty()) {
    dump(info[0], type, appending);
    for (int i = 1; i < info.size(); ++i) {
      dump(info[1], type, true);
    }
  }
}

QTabWidget* ZFlyEmMessageWidget::getTab()
{
  return this;
}
