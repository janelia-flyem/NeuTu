#include "zcheckboxgroup.h"

#include <QHBoxLayout>
#include <QCheckBox>

ZCheckBoxGroup::ZCheckBoxGroup(QWidget* parent)
  : ZGroupVisWidget(parent)
{
  setLayout(new QHBoxLayout(this));
  layout()->setMargin(0);
  layout()->setSpacing(0);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void ZCheckBoxGroup::setCheckBox(int index, const CheckBoxConfig &config)
{
  while (layout()->takeAt(0));

  if (m_checkBoxList.size() <= index) {
    m_checkBoxList.resize(index + 1);
  }
  if (m_checkBoxList[index] != NULL) {
    assumeVisible(m_checkBoxList[index], false);
    m_checkBoxList[index]->deleteLater();
  }
  QCheckBox *widget = new QCheckBox(config.m_text);
  assumeVisible(widget, true);
  m_checkBoxList[index] = widget;
  if (config.m_receiver && config.m_slot) {
    connect(widget, SIGNAL(toggled(bool)), config.m_receiver, config.m_slot);
  }
  widget->setChecked(config.m_defaultValue);

  foreach (auto widget, m_checkBoxList) {
    if (widget) {
      layout()->addWidget(widget);
    }
  }
}

QCheckBox* ZCheckBoxGroup::getCheckBox(int index) const
{
  if (index >= 0 && index < m_checkBoxList.size()) {
    return m_checkBoxList[index];
  }

  return nullptr;
}

void ZCheckBoxGroup::processCheckBox(
    int index, std::function<void(QCheckBox*)> f)
{
  if (f) {
    QCheckBox *widget = getCheckBox(index);
    if (widget) {
      f(widget);
    }
  }
}

ZCheckBoxGroup::CheckBoxConfigBuilder::CheckBoxConfigBuilder(
    const QString &text)
{
  m_config.m_text = text + " |  ";
}

ZCheckBoxGroup::CheckBoxConfigBuilder::operator CheckBoxConfig() const
{
  return m_config;
}

ZCheckBoxGroup::CheckBoxConfigBuilder&
ZCheckBoxGroup::CheckBoxConfigBuilder::connectTo(
    QObject *receiver, const char *slot)
{
  m_config.m_receiver = receiver;
  m_config.m_slot = slot;

  return *this;
}

ZCheckBoxGroup::CheckBoxConfigBuilder&
ZCheckBoxGroup::CheckBoxConfigBuilder::checkedByDefault(bool on)
{
  m_config.m_defaultValue = on;

  return *this;
}
