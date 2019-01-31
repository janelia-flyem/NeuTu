#include "zwidgetsgroup.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QToolBox>

//#include "z3dcameraparameter.h"
//#include "z3dtransformparameter.h"
#include "zparameter.h"

namespace {

bool widgetGroupPtVisibleLevelLessThan(const std::shared_ptr<ZWidgetsGroup>& s1,
                                       const std::shared_ptr<ZWidgetsGroup>& s2)
{
  return s1->visibleLevel() < s2->visibleLevel();
}

} // namespace

ZWidgetsGroup::ZWidgetsGroup(QWidget& widget, int visibleLevel)
  : m_type(Type::Widget)
  , m_groupName("This is not a group")
  , m_widget(&widget)
  , m_visibleLevel(visibleLevel)
{
}

ZWidgetsGroup::ZWidgetsGroup(const QString& groupName, int visibleLevel)
  : m_type(Type::Group)
  , m_groupName(groupName)
  , m_visibleLevel(visibleLevel)
{
}

ZWidgetsGroup::ZWidgetsGroup(ZParameter& parameter, int visibleLevel)
  : m_type(Type::Parameter)
  , m_groupName("This is not a group")
  , m_parameter(&parameter)
  , m_visibleLevel(visibleLevel)
{
}

std::vector<ZParameter*> ZWidgetsGroup::getParameterList()
{
  std::vector<ZParameter*> res;
  if (m_type == Type::Parameter) {
    res.push_back(m_parameter);
  } else if (m_type == Type::Group) {
    if (!m_isSorted) {
      sortChildGroups();
    }
    for (const auto& childGroup : m_childGroups) {
      std::vector<ZParameter*> tmpRes = childGroup->getParameterList();
      for (auto pp : tmpRes) {
        res.push_back(pp);
      }
    }
  }
  return res;
}

const std::vector<std::shared_ptr<ZWidgetsGroup>>& ZWidgetsGroup::getChildGroups()
{
  if (!m_isSorted)
    sortChildGroups();
  return m_childGroups;
}

void ZWidgetsGroup::addChild(QWidget& widget, int visibleLevel)
{
  addChild(std::make_shared<ZWidgetsGroup>(widget, visibleLevel));
}

void ZWidgetsGroup::addChild(ZParameter& parameter, int visibleLevel)
{
  addChild(std::make_shared<ZWidgetsGroup>(parameter, visibleLevel));
}

void ZWidgetsGroup::addChild(std::shared_ptr<ZWidgetsGroup> child, bool atEnd)
{
  if (atEnd) {
    m_childGroups.push_back(child);
  } else {
    m_childGroups.insert(m_childGroups.begin(), child);
  }
  connect(child.get(), &ZWidgetsGroup::widgetsGroupChanged,
          this, &ZWidgetsGroup::widgetsGroupChanged);
  connect(child.get(), &ZWidgetsGroup::requestAdvancedWidget,
          this, &ZWidgetsGroup::requestAdvancedWidget);
  m_isSorted = false;
}

void ZWidgetsGroup::removeAllChildren()
{
  for (const auto& childGroup : m_childGroups) {
    childGroup->disconnect(this);
  }
  m_childGroups.clear();
}

bool ZWidgetsGroup::eraseChild(
    std::vector<std::shared_ptr<ZWidgetsGroup>>::iterator iter)
{
  if (iter != m_childGroups.end()) {
    m_childGroups.erase(iter, m_childGroups.end());
    return true;
  }

  return false;
}

bool ZWidgetsGroup::removeChild(const ZParameter& para)
{
  qDebug() << "removing child:" << para.name();

  auto pred = [&para, this](const std::shared_ptr<ZWidgetsGroup>& child) {
    if (child->m_type == Type::Parameter && child->m_parameter == &para) {
      qDebug() << "Child removed:" << para.name();
      child->disconnect(this);
      return true;
    }
    return false;
  };

  return eraseChild(std::remove_if(m_childGroups.begin(), m_childGroups.end(),
                                   pred));

}

bool ZWidgetsGroup::removeChild(const std::shared_ptr<ZWidgetsGroup>& childIn)
{
  qDebug() << "removing child:" << childIn->getGroupName();

  auto pred = [&childIn, this](const std::shared_ptr<ZWidgetsGroup>& child) {
    if (child->m_type == Type::Group && child == childIn) {
      qDebug() << "Child removed:" << child->getGroupName();
      child->disconnect(this);
      return true;
    }
    return false;
  };

  return eraseChild(std::remove_if(m_childGroups.begin(), m_childGroups.end(),
                                   pred));
}

QWidget* ZWidgetsGroup::createWidget(bool createBasic, bool scroll, QLabel* label)
{
  QLayout* lw = createLayout(createBasic);
  // if is boxLayout, add strech to fill the space
  if (QBoxLayout* blo = qobject_cast<QBoxLayout*>(lw)) {
    blo->addStretch();
    //
    if (label) {
      blo->insertWidget(0, label);
      blo->insertSpacing(1, 20);
    }
  }
  QWidget* widget = new QWidget();
  widget->setLayout(lw);
  if (scroll) {
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setWidget(widget);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setContentsMargins(0, 0, 0, 0);

    //sa->setVisible(isVisible());

    return sa;
  } else {
    return widget;
  }
}

QLayout* ZWidgetsGroup::createWidgetLayout()
{
  QHBoxLayout* hbl = new QHBoxLayout;
  hbl->addWidget(m_widget);

  return hbl;
}

QLayout* ZWidgetsGroup::createParameterLayout()
{
  QHBoxLayout* hbl = new QHBoxLayout;
//  if (qobject_cast<Z3DCameraParameter*>(m_parameter) ||
//    qobject_cast<Z3DTransformParameter*>(m_parameter)) {
  if (m_parameter->name().isEmpty()) {
    QWidget* wg = m_parameter->createWidget();
    hbl->addWidget(wg);
  } else {
    QLabel* label = m_parameter->createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    label->setWordWrap(true);
    hbl->addWidget(label);
    QWidget* wg = m_parameter->createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hbl->addWidget(wg);
  }

  return hbl;
}

QLayout* ZWidgetsGroup::createGroupLayout(bool createBasic)
{
  QVBoxLayout* vbl = new QVBoxLayout;
  if (!m_isSorted) {
    sortChildGroups();
  }

  if (createBasic) {
    size_t i;
    for (i = 0; i < m_childGroups.size() &&
                m_childGroups[i]->m_visibleLevel <= m_cutOffbetweenBasicAndAdvancedLevel; ++i) {
      QLayout* lw = m_childGroups[i]->createLayout(true);
      if (m_childGroups[i]->isGroup()) {
        QGroupBox* groupBox = new QGroupBox(m_childGroups[i]->getGroupName());
        groupBox->setLayout(lw);
        vbl->addWidget(groupBox);
      } else
        vbl->addLayout(lw);
    }
    if (i < m_childGroups.size()) {
      QPushButton* pb = new QPushButton("Advanced...");
      pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      vbl->addWidget(pb, 0, Qt::AlignHCenter | Qt::AlignVCenter);
      connect(pb, &QPushButton::clicked, this, &ZWidgetsGroup::emitRequestAdvancedWidgetSignal);
    }
  } else {
    if (m_useToolBoxStyle) {
      QToolBox* toolBox = new QToolBox();
      for (const auto& childGroup : m_childGroups) {
        if (m_useToolBoxStyle) {
          if (childGroup->isGroup()) {
            QWidget* wg = childGroup->createWidget(false, false);
            toolBox->addItem(wg, childGroup->getGroupName());
            // fully expand page, no scroll bar
            qobject_cast<QScrollArea*>(
                  wg->parentWidget()->parentWidget())->setSizePolicy(
                  QSizePolicy::Preferred, QSizePolicy::Fixed);
          } else {
            QLayout* lw = childGroup->createLayout(false);
            vbl->addLayout(lw);
          }
        } else {
          if (childGroup->isGroup()) {
            QGroupBox* groupBox = new QGroupBox(childGroup->getGroupName());
            QLayout* lw = childGroup->createLayout(false);
            groupBox->setLayout(lw);
            vbl->addWidget(groupBox);
          } else {
            QLayout* lw = childGroup->createLayout(false);
            vbl->addLayout(lw);
          }
        }
      }
      vbl->addWidget(toolBox);
    } else {
      for (const auto& childGroup : m_childGroups) {
        if (childGroup->isGroup()) {
          QGroupBox* groupBox = new QGroupBox(childGroup->getGroupName());
          QLayout* lw = childGroup->createLayout(false);
          groupBox->setLayout(lw);
          vbl->addWidget(groupBox);
        } else {
          QLayout* lw = childGroup->createLayout(false);
          vbl->addLayout(lw);
        }
      }
    }
  }
  return vbl;
}


QLayout* ZWidgetsGroup::createLayout(bool createBasic)
{
  QLayout *layout = nullptr;
  switch (m_type) {
  case Type::Widget:
    layout = createWidgetLayout();
    break;
  case Type::Parameter:
    layout = createParameterLayout();
    break;
  default: /*case GROUP:*/
    layout = createGroupLayout(createBasic);
    break;
  }

  return layout;
}

bool ZWidgetsGroup::operator<(const ZWidgetsGroup& other) const
{
  return m_visibleLevel < other.m_visibleLevel;
}

void ZWidgetsGroup::sortChildGroups()
{
  std::stable_sort(m_childGroups.begin(), m_childGroups.end(),
                   widgetGroupPtVisibleLevelLessThan);
  m_isSorted = true;
}

void ZWidgetsGroup::emitRequestAdvancedWidgetSignal()
{
  emit requestAdvancedWidget(m_groupName);
}

void ZWidgetsGroup::emitWidgetsGroupChangedSignal()
{
  emit widgetsGroupChanged();
}
