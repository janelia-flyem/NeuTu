#include "taskprotocoltaskfactory.h"

#include <map>
#include <numeric>

namespace {
  std::map<QString, TaskProtocolTaskFactory::JsonCreator> s_typeToJsonCreator;
  std::map<QString, TaskProtocolTaskFactory::GuiCreator> s_typeToGuiCreator;
}

TaskProtocolTaskFactory& TaskProtocolTaskFactory::getInstance()
{
  static TaskProtocolTaskFactory singleton;
  return singleton;
}

void TaskProtocolTaskFactory::registerJsonCreator(QString taskType,
                                                  JsonCreator creator)
{
  s_typeToJsonCreator[taskType] = creator;
}

TaskProtocolTask* TaskProtocolTaskFactory::createFromJson(QString taskType,
                                                          QJsonObject json,
                                                          ZFlyEmBody3dDoc* doc) const
{
  auto it = s_typeToJsonCreator.find(taskType);
  if (it != s_typeToJsonCreator.end()) {
    JsonCreator creator = it->second;
    return creator(json, doc);
  } else {
    return nullptr;
  }
}

void TaskProtocolTaskFactory::registerGuiCreator(QString taskType,
                                                  GuiCreator creator)
{
  s_typeToGuiCreator[taskType] = creator;
}

std::vector<QString> TaskProtocolTaskFactory::registeredGuiTaskTypes() const
{
  std::vector<QString> result;
  for (auto it : s_typeToGuiCreator) {
    result.push_back(it.first);
  }
  return result;
}

TaskProtocolTask* TaskProtocolTaskFactory::createFromGui(QString taskType,
                                                          ZFlyEmBody3dDoc* doc) const
{
  auto it = s_typeToGuiCreator.find(taskType);
  if (it != s_typeToGuiCreator.end()) {
    GuiCreator creator = it->second;
    return creator(doc);
  } else {
    return nullptr;
  }
}

TaskProtocolTaskFactory::TaskProtocolTaskFactory()
{
}
