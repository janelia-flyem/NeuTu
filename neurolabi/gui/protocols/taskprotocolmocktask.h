#ifndef TASKPROTOCOLMOCKTASK_H
#define TASKPROTOCOLMOCKTASK_H

#include "taskprotocoltask.h"

class TaskProtocolTaskMock: public TaskProtocolTask
{
  Q_OBJECT

public:
  TaskProtocolTaskMock();
  ~TaskProtocolTaskMock() override {}

  virtual QString taskType() const { return ""; }
  virtual QString actionString() { return ""; }
  virtual QString targetString() { return ""; }

private:
  virtual bool loadSpecific(QJsonObject /*json*/) { return false; }
  virtual QJsonObject addToJson(QJsonObject json) { return json; }
};
#endif // TASKPROTOCOLMOCKTASK_H
