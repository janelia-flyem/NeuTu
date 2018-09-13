#ifndef PROTOCOLTASKFACTORY_H
#define PROTOCOLTASKFACTORY_H

#include <QString>

class ProtocolTaskFactory
{
public:
  ProtocolTaskFactory();

public:
  static const QString TASK_BODY_REVIEW;
  static const QString TASK_BODY_HISTORY;
  static const QString TASK_BODY_CLEAVE;
  static const QString TASK_BODY_MERGE;
  static const QString TASK_SPLIT_SEEDS;
  static const QString TASK_TEST;
};

#endif // PROTOCOLTASKFACTORY_H
