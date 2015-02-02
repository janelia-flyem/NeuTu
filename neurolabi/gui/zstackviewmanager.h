#ifndef ZSTACKVIEWMANAGER_H
#define ZSTACKVIEWMANAGER_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QSet>
#include <QMultiMap>

class QWiget;
class ZStackFrame;
class Z3DWindow;
class ZStackViewParam;

class ZStackViewManager : public QObject
{
  Q_OBJECT
public:
  explicit ZStackViewManager(QObject *parent = 0);

  void registerWindowPair(ZStackFrame *sender, Z3DWindow *receiver);
  void registerWindowPair(Z3DWindow *sender, ZStackFrame *receiver);
  void registerWindowPair(ZStackFrame *sender, ZStackFrame *receiver);

  void print() const;

  bool hasWidget(QWidget *widget);

signals:

public slots:
  void removeWidget();
  void removeWidget(QWidget *widget);
  void updateView(const ZStackViewParam &param);

private slots:
  void slotTest();

private:
  void registerWidgetPair(QWidget *sender, QWidget *receiver);
  void registerWindow(ZStackFrame *window);
  void registerWindow(Z3DWindow *window);

private:
  QMultiMap<QWidget*, QWidget*> m_windowGraph;
};

#endif // ZSTACKVIEWMANAGER_H
