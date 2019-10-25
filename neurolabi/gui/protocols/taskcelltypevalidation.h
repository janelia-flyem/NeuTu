#ifndef TASKCELLTYPEVALIDATION_H
#define TASKCELLTYPEVALIDATION_H

#include "protocols/taskreview.h"

class TaskCellTypeValidation : public TaskReview
{
  Q_OBJECT
public:
  TaskCellTypeValidation(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  // For use with TaskProtocolTaskFactory.
  static QString taskTypeStatic();
  static TaskCellTypeValidation* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

  virtual QString taskType() const override;
  virtual QString actionString() override;

  virtual void beforeNext() override;
  virtual void beforePrev() override;

protected:
  virtual bool includeExtendedResults() const override;
  virtual bool onLoadedZoomToFitSmaller() const override;

  virtual std::string getOutputInstanceName(const ZDvidTarget &dvidTarget) const override;

  virtual QString labelForDoButton() const override;
  virtual QString labelForDontButton() const override;

  virtual QString valueResultDo() const override;
  virtual QString valueResultDont() const override;

  virtual QLayout *customBottomWidgets(QWidget *parent) override;

private slots:
  void onShowBody();
  void onShowAllBodies();
  void onCycleShowBody();
  void onCycleShowAllBodies();

private:
  QRadioButton *m_showBothButton;
  QRadioButton *m_showAButton;
  QRadioButton *m_showBButton;
};

#endif // TASKCELLTYPEVALIDATION_H
