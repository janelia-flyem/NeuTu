#include "taskmergereview.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

namespace {
  static const QString VALUE_TASKTYPE = "merge review";
}

//

TaskMergeReview::TaskMergeReview(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
  : TaskReview(json, bodyDoc)
{
  init();
}

QString TaskMergeReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskMergeReview* TaskMergeReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskMergeReview(json, bodyDoc);
}

QString TaskMergeReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskMergeReview::actionString()
{
  return "Merge review:";
}

std::string TaskMergeReview::getOutputInstanceName(const ZDvidTarget &dvidTarget) const
{
  return dvidTarget.getBodyLabelName() + "_mergeReview";
}

QString TaskMergeReview::labelForDoButton() const
{
  return "Merge";
}

QString TaskMergeReview::labelForDontButton() const
{
  return "Don't merge";
}

QString TaskMergeReview::labelForDoMajorButton() const
{
  return "Merge major only";
}


QString TaskMergeReview::valueResultDo() const
{
  return "merge";
}

QString TaskMergeReview::valueResultDont() const
{
  return "dontMerge";
}

QString TaskMergeReview::valueResultDoMajor() const
{
  return "mergeMajor";
}

