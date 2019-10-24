#include "taskcelltypevalidation.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

namespace {
  static const QString VALUE_TASKTYPE = "type review";
}

//

TaskCellTypeValidation::TaskCellTypeValidation(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
  : TaskReview(json, bodyDoc)
{
  init();
}

QString TaskCellTypeValidation::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskCellTypeValidation* TaskCellTypeValidation::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskCellTypeValidation(json, bodyDoc);
}

QString TaskCellTypeValidation::taskType() const
{
  return taskTypeStatic();
}

QString TaskCellTypeValidation::actionString()
{
  return "Cell type validation:";
}

bool TaskCellTypeValidation::includeExtendedResults() const
{
  return false;
}

bool TaskCellTypeValidation::onLoadedZoomToFitSmaller() const
{
  return false;
}

QString TaskCellTypeValidation::labelForDoButton() const
{
  return "Group";
}

QString TaskCellTypeValidation::labelForDontButton() const
{
  return "Don't group";
}

std::string TaskCellTypeValidation::getOutputInstanceName(const ZDvidTarget &dvidTarget) const
{
  return dvidTarget.getBodyLabelName() + "_cellTypeValidation";
}

QString TaskCellTypeValidation::valueResultDo() const
{
  return "group";
}

QString TaskCellTypeValidation::valueResultDont() const
{
  return "dontGroup";
}


