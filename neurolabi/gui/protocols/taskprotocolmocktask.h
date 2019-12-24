#ifndef TASKPROTOCOLMOCKTASK_H
#define TASKPROTOCOLMOCKTASK_H

#include "taskprotocoltask.h"

class TaskProtocolTaskMock: public TaskProtocolTask
{
  Q_OBJECT

public:
  TaskProtocolTaskMock();
  ~TaskProtocolTaskMock() override {}

  virtual QString taskType() const override { return ""; }
  virtual QString actionString() override { return ""; }
  virtual QString targetString() override { return ""; }

private:
  virtual bool loadSpecific(QJsonObject /*json*/) override { return false; }
  virtual QJsonObject addToJson(QJsonObject json) override { return json; }
};
#endif // TASKPROTOCOLMOCKTASK_H
