#include "taskcelltypevalidation.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QRadioButton>

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

QLayout *TaskCellTypeValidation::customBottomWidgets(QWidget *parent)
{
  QLabel *showLabel = new QLabel("Show bodies: ", parent);

  m_showBothButton = new QRadioButton("Both", parent);
  m_showBothButton->setChecked(true);
  connect(m_showBothButton, SIGNAL(toggled(bool)), this, SLOT(onShowAllBodies()));

  m_showAButton = new QRadioButton("A", parent);
  connect(m_showAButton, SIGNAL(toggled(bool)), this, SLOT(onShowBody()));

  m_showBButton = new QRadioButton("B", parent);
  connect(m_showBButton, SIGNAL(toggled(bool)), this, SLOT(onShowBody()));

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(showLabel);
  layout->addWidget(m_showBothButton);
  layout->addWidget(m_showAButton);
  layout->addWidget(m_showBButton);

  QButtonGroup *group = new QButtonGroup(parent);
  group->addButton(m_showBothButton);
  group->addButton(m_showAButton);
  group->addButton(m_showBButton);

  QAction *cycleAction = new QAction("Cycle Through Bodies", parent);
  cycleAction->setShortcut(Qt::Key_Space);
  getTaskMenu()->addAction(cycleAction);
  connect(cycleAction, SIGNAL(triggered()), this, SLOT(onCycleShowBody()));

  QAction *showAllAction = new QAction("Show All Bodies", parent);
  showAllAction->setShortcut(Qt::SHIFT + Qt::Key_Space);
  getTaskMenu()->addAction(showAllAction);
  connect(showAllAction, SIGNAL(triggered()), this, SLOT(onCycleShowAllBodies()));

  return layout;
}

void TaskCellTypeValidation::onShowBody()
{
  std::size_t which = m_showAButton->isChecked() ? 0 : 1;
  hideBody(which);
}

void TaskCellTypeValidation::onShowAllBodies()
{
  hideNone();
}

void TaskCellTypeValidation::onCycleShowBody()
{
  if (m_showBothButton->isChecked()) {
    m_showAButton->setChecked(true);
  } else if (m_showAButton->isChecked()) {
    m_showBButton->setChecked(true);
  } else if (m_showBButton->isChecked()) {
    m_showAButton->setChecked(true);
  }
}

void TaskCellTypeValidation::onCycleShowAllBodies()
{
  if (!m_showBothButton->isChecked()) {
    m_showBothButton->setChecked(true);
  }
}

