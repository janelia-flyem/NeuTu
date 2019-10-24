#ifndef TASKCELLTYPEVALIDATION_H
#define TASKCELLTYPEVALIDATION_H

#include "protocols/taskreview.h"

class TaskCellTypeValidation : public TaskReview
{
public:
  TaskCellTypeValidation(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  // For use with TaskProtocolTaskFactory.
  static QString taskTypeStatic();
  static TaskCellTypeValidation* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  virtual QString taskType() const override;
  virtual QString actionString() override;

protected:
  virtual bool includeExtendedResults() const override;
  virtual bool onLoadedZoomToFitSmaller() const override;

  virtual std::string getOutputInstanceName(const ZDvidTarget &dvidTarget) const override;

  virtual QString labelForDoButton() const override;
  virtual QString labelForDontButton() const override;

  virtual QString valueResultDo() const override;
  virtual QString valueResultDont() const override;
};

#endif // TASKCELLTYPEVALIDATION_H
