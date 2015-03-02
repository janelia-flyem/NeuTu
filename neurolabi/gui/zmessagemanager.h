#ifndef ZMESSAGEMANAGER_H
#define ZMESSAGEMANAGER_H

#include <QObject>

class ZMessage;
class QWidget;
class ZMessageProcessor;

class ZMessageManager : public QObject
{
  Q_OBJECT
public:
  explicit ZMessageManager(QObject *parent = 0);
  void processMessage(ZMessage *message);
  void dispatchMessage(ZMessage *message);
  void reportMessage(ZMessage *message);

  void registerWidget(QWidget *widget);



signals:

public slots:
  void detachWidget();

private:
  QWidget *m_widget;
  ZMessageProcessor *m_processor;
};

#endif // ZMESSAGEMANAGER_H
