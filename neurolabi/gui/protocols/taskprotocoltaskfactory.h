#ifndef TASKPROTOCOLTASKFACTORY_H
#define TASKPROTOCOLTASKFACTORY_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <functional>
#include <vector>

class TaskProtocolTask;
class ZFlyEmBody3dDoc;

class TaskProtocolTaskFactory
{
public:
  static TaskProtocolTaskFactory& getInstance();

  //

  typedef std::function<TaskProtocolTask*(QJsonObject, ZFlyEmBody3dDoc*)> JsonCreator;

  void registerJsonCreator(QString taskType, JsonCreator creator);

  TaskProtocolTask* createFromJson(QString taskType, QJsonObject json, ZFlyEmBody3dDoc* doc) const;

  //

  typedef std::function<QJsonArray(ZFlyEmBody3dDoc*)> GuiCreator;

  void registerGuiCreator(QString menuLabel, GuiCreator creator);

  std::vector<QString> registeredGuiMenuLabels() const;

  QJsonArray createFromGui(QString menuLabel, ZFlyEmBody3dDoc* doc) const;

private:
  TaskProtocolTaskFactory();
};

#endif // TASKPROTOCOLTASKFACTORY_H
