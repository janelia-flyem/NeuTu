#ifndef TASKPROTOCOLTASKFACTORY_H
#define TASKPROTOCOLTASKFACTORY_H

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

  typedef std::function<TaskProtocolTask*(ZFlyEmBody3dDoc*)> GuiCreator;

  void registerGuiCreator(QString taskType, GuiCreator creator);

  std::vector<QString> registeredGuiTaskTypes() const;

  TaskProtocolTask* createFromGui(QString taskType, ZFlyEmBody3dDoc* doc) const;

private:
  TaskProtocolTaskFactory();
};

#endif // TASKPROTOCOLTASKFACTORY_H
