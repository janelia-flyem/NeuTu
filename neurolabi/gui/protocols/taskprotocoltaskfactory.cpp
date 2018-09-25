#include "taskprotocoltaskfactory.h"

#include <map>
#include <numeric>

namespace {
  std::map<QString, TaskProtocolTaskFactory::JsonCreator> s_typeToJsonCreator;
  std::map<QString, TaskProtocolTaskFactory::GuiCreator> s_menuLabelToGuiCreator;
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

void TaskProtocolTaskFactory::registerGuiCreator(QString menuLabel,
                                                 GuiCreator creator)
{
  s_menuLabelToGuiCreator[menuLabel] = creator;
}

std::vector<QString> TaskProtocolTaskFactory::registeredGuiMenuLabels() const
{
  std::vector<QString> result;
  for (auto it : s_menuLabelToGuiCreator) {
    result.push_back(it.first);
  }
  return result;
}

QJsonArray TaskProtocolTaskFactory::createFromGui(QString menuLabel,
                                                  ZFlyEmBody3dDoc* doc) const
{
  auto it = s_menuLabelToGuiCreator.find(menuLabel);
  if (it != s_menuLabelToGuiCreator.end()) {
    GuiCreator creator = it->second;
    return creator(doc);
  } else {
    return QJsonArray();
  }
}

TaskProtocolTaskFactory::TaskProtocolTaskFactory()
{
}
