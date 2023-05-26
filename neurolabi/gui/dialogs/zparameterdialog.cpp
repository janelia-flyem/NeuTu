#include "zparameterdialog.h"

#include <iostream>

#include <QHBoxLayout>
#include <QLayoutItem>
#include <QLabel>

#include "common/debug.h"
#include "logging/zlog.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zstring.h"

#include "zjsonobjectparser.h"
#include "qt/gui/utilities.h"
#include "widgets/zparameter.h"
#include "widgets/zstringparameter.h"
#include "widgets/zoptionparameter.h"
#include "widgets/znumericparameter.h"
#include "zwidgetfactory.h"

ZParameterDialog::ZParameterDialog(QWidget *parent) : QDialog(parent)
{
  m_mainLayout = new QVBoxLayout;
  setLayout(m_mainLayout);
  resetParameterLayout();

  m_auxLayout = new QVBoxLayout;
  m_mainLayout->addLayout(m_auxLayout);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addSpacerItem(ZWidgetFactory::MakeHSpacerItem());

  QPushButton *cancelButton = new QPushButton(this);
  buttonLayout->addWidget(cancelButton);
  cancelButton->setText("Cancel");
  cancelButton->setAutoDefault(false);
  cancelButton->setDefault(false);

  QPushButton *okButton = new QPushButton(this);
  buttonLayout->addWidget(okButton);
  okButton->setText("OK");
  okButton->setAutoDefault(true);
  okButton->setDefault(true);

  m_mainLayout->addLayout(buttonLayout);

//    m_layout->addSpacerItem(ZWidgetFactory::MakeVSpacerItem());

  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  m_label = new QLabel(this);
  m_label->setTextFormat(Qt::RichText);
  m_label->hide();
}

int ZParameterDialog::exec()
{
  build();
  return QDialog::exec();
}

bool ZParameterDialog::buildRequied() const
{
  return (m_prevGeneration != m_currentGeneration);
}

void ZParameterDialog::updateGeneration()
{
  ++m_currentGeneration;
}

ZParameterDialog::Param ZParameterDialog::getParamBundle(
    const QString &name) const
{
  return m_parameterMap.value(name);
}

ZParameterDialog::Param ZParameterDialog::setParam(
    const QString &name, std::function<void (Param &)> f, bool forcing)
{
  if (m_parameterMap.contains(name) || forcing) {
    f(m_parameterMap[name]);
  }

  return getParamBundle(name);
}

ZParameter* ZParameterDialog::getParameter(const QString &name) const
{
  return m_parameterMap.value(name).parameter;
}

void ZParameterDialog::addAuxWidget(QWidget *widget)
{
  if (widget) {
    m_auxLayout->addWidget(widget);
  }
}

void ZParameterDialog::configure(const ZJsonObject &config)
{
  m_parameterMap.clear();

  if (config.hasKey("collection")) {
    ZJsonObject collection(config.value("collection"));
    collection.forEachValue([&](const std::string &key, ZJsonValue v) {
      ZJsonObject fieldJson(v);
      ZJsonObject editElement(fieldJson.value("editElement"));
      std::string type =
          ZJsonObjectParser::GetValue(editElement, "type", "input");
      QStringList options;
      if (editElement.hasKey("options")) {
        ZJsonArray optionArray(editElement.value("options"));
        optionArray.forEachString([&](const std::string &option) {
          options.append(option.c_str());
        });
      }
      if (type == "input") {
        if (!options.isEmpty()) {
          addStringParameter(key.c_str(), options);
        } else {
          addStringParameter(key.c_str());
        }
      } else if (type == "select") {
        addStringParameter(key.c_str(), options);
      } else if (type == "checkbox") {
        addBoolParameter(key.c_str());
      }
      if (editElement.isEmpty()) {
        auto param = getParameter(key.c_str());
        if (param) {
          param->setEnabled(false);
        }
      }
    });
  }

  m_shape.clear();
  if (config.hasKey("shape")) {
    ZJsonArray shape(config.value("shape"));
    shape.forEachString([&](const std::string &str) {
      m_shape.append(str.c_str());
    });
  }

  updateGeneration();
}

void ZParameterDialog::setLabel(const QString &label, const QString &tooltip)
{
  if (m_label) {
    m_label->setText(label);
    m_label->setVisible(!label.isEmpty());
    if (!tooltip.isEmpty()) {
        m_label->setToolTip(tooltip);
    } else {
        m_label->setToolTip("");
    }
  } else {
    ZWARN(neutu::TOPIC_NULL) << "Cannot set uninitialized label.";
  }
}

void ZParameterDialog::resetParameterLayout()
{
  if (m_parameterLayout) {
    m_parameterLayout->removeWidget(m_label);
    neutu::ClearLayout(m_parameterLayout);
    delete m_parameterLayout;
  }

  m_parameterLayout = new QVBoxLayout;
  m_mainLayout->insertLayout(0, m_parameterLayout);
}

void ZParameterDialog::build()
{
  if (buildRequied()) {
    m_prevGeneration = m_currentGeneration;

#ifdef _DEBUG_
    std::cout << __FUNCTION__ << ": building dialog" << std::endl;
#endif

    resetParameterLayout();

    m_parameterLayout->addWidget(m_label);

    foreach(const QString &name, m_shape) {
      ZParameter *param = getParameter(name);
      if (param) {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(param->createNameLabel(this));
        layout->addWidget(param->createWidget(this));
        layout->addSpacerItem(ZWidgetFactory::MakeHSpacerItem());
        m_parameterLayout->addLayout(layout);
      }
    }
  }
}

void ZParameterDialog::addParameter(
    ZParameter *param, const QString &valueType, bool appending)
{
  if (param) {
    param->setParent(this);

    QString name = param->name();

    if (appending && m_shape.indexOf(name) >= 0) {
      removeParameter(name);
      m_shape.append(name);
    } else {
      auto param = getParameter(name);
      if (param) {
        param->deleteLater();
      }
    }

    m_parameterMap[name].parameter = param;
    m_parameterMap[name].valueType = valueType;

    updateGeneration();
  }
}

void ZParameterDialog::addStringParameter(const QString &name, bool appending)
{
  if (!name.isEmpty()) {
    ZStringParameter *param = new ZStringParameter(name, this);
    addParameter(param, "string", appending);
    setDefaultValue(name, QString(""));
  }
}

QStringList ZParameterDialog::updateOptions(
    ZStringStringOptionParameter *param, const QStringList &options,
    const QString newOption)
{
  QStringList newOptions;
  if (!newOption.isEmpty() && !options.contains(newOption)) {
    newOptions.append(newOption);
  }
  newOptions.append("");
  newOptions.append(options);

  const QList<QString> &oldOptions = param->options();

  bool updateNeeded = false;
  if (oldOptions.size() == newOptions.size()) {
    for (int i = 0; i < oldOptions.size(); ++i) {
      if (oldOptions[i] != newOptions[i]) {
        updateNeeded = true;
      }
    }
  } else {
    updateNeeded = true;
  }

  if (updateNeeded) {
    param->clearOptions();
    param->clearValue();
    foreach (const QString &option, newOptions) {
      param->addOptionWithData({option, option});
    }
    updateGeneration();
  }

  return newOptions;
}

void ZParameterDialog::addStringParameter(
    const QString &name, const QStringList &options, bool appending)
{
  if (!name.isEmpty()) {
    ZStringStringOptionParameter *param =
        new ZStringStringOptionParameter(name, this);
    auto newOptions = updateOptions(param, options, "");
    addParameter(param, "string", appending);
    if (!newOptions.isEmpty()) {
      setDefaultValue(name, newOptions[0]);
    }
    m_parameterMap[name].defaultOptions = options;
  }
}

void ZParameterDialog::addIntParameter(
    const QString &name, int defaultValue, int min, int max)
{
  if (!name.isEmpty()) {
    ZIntParameter *param = new ZIntParameter(name, defaultValue, min, max, this);
    param->setStyle("SPINBOX");
    addParameter(param, "int");
    setDefaultValue(name, defaultValue);
  }
}

void ZParameterDialog::addBoolParameter(const QString &name)
{
  if (!name.isEmpty()) {
    ZBoolParameter *param = new ZBoolParameter(name, this);
    addParameter(param, "bool");
    setDefaultValue(name, false);
  }
}

/* It doesn't work for adjusting the dialog size.
void ZParameterDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
  setFixedHeight(sizeHint().height());
  adjustSize();
}
*/

void ZParameterDialog::removeParameter(const QString &name)
{
  if (!name.isEmpty()) {
    m_shape.removeAll(name);
    ZParameter *oldParam = getParameter(name);
    if (oldParam) {
      oldParam->deleteLater();
    }

    updateGeneration();
  }
}

QString ZParameterDialog::getStringValue(const QString &name) const
{
  try {
    if (m_parameterMap.contains(name)) {
      const auto &p = m_parameterMap.value(name);
      ZStringStringOptionParameter *param =
          dynamic_cast<ZStringStringOptionParameter*>(p.parameter);
      if (param) {
        return param->associatedData();
      } else {
        return getGenericValue<QString>(name);
      }
    }

  } catch (...) {
  }

  return "";
}

bool ZParameterDialog::getBoolValue(const QString &name) const
{
  try {
    return getGenericValue<bool>(name);
  }  catch (...) {
  }

  return false;
}

int ZParameterDialog::getIntValue(const QString &name) const
{
  ZStringIntOptionParameter *optionParam =
      qobject_cast<ZStringIntOptionParameter*>(getParameter(name));
  if (optionParam) {
    return optionParam->associatedData();
  }

  try {
    return getGenericValue<int>(name);
  } catch (...) {
  }

  return 0;
}

void ZParameterDialog::setValue(const QString &name, const QVariant &value)
{
  if (!value.isNull()) {
    Param p = m_parameterMap.value(name);
    if (p.valueType == "int") {
      setValue(name, value.toInt());
    } else if (p.valueType == "string") {
      ZStringStringOptionParameter *optionParam =
          qobject_cast<ZStringStringOptionParameter*>(getParameter(name));
      if (optionParam) {
        QString str = value.toString();
        if (!p.defaultOptions.contains(str)) {
          optionParam->clearOptions();
          updateOptions(optionParam, p.defaultOptions, str);
        }
      }

      setValue(name, value.toString());
    } else if (p.valueType == "bool") {
      setValue(name, value.toBool());
    }
  }
}

void ZParameterDialog::setValue(const QString &name, const QString &value)
{
  Param p = m_parameterMap.value(name);

  ZStringStringOptionParameter *optionParam =
      qobject_cast<ZStringStringOptionParameter*>(getParameter(name));
  if (optionParam) {
    QStringList newOptions =
        updateOptions(optionParam, p.defaultOptions, value);
    if (!newOptions.isEmpty()) {
      p.defaultValue = newOptions[0];
    }
    optionParam->set(value);
  } else {
    setValue<QString>(name, value);
  }
}

void ZParameterDialog::setValue(const QString &name, const char *value)
{
  setValue(name, QString(value));
}

void ZParameterDialog::postProcess(ZJsonObject &/*obj*/) const
{
}

ZJsonObject ZParameterDialog::toJsonObject() const
{
  ZJsonObject json;

  foreach(const QString &name, m_shape) {
    Param p = m_parameterMap.value(name);
    if (p.parameter) {
      if (p.valueType == "string") {
        json.setEntry(name.toStdString(), getStringValue(name).toStdString());
      } else if (p.valueType == "int") {
        json.setEntry(name.toStdString(), getIntValue(name));
      } else if (p.valueType == "bool") {
        json.setEntry(name.toStdString(), getBoolValue(name));
      }
    }
  }

  HLDEBUG_FUNC("annotate body")
      << " Annotation before post process: "  <<json.dumpString(0) << std::endl;
  postProcess(json);
  HLDEBUG_FUNC("annotate body")
      << " Annotation after post process: "  <<json.dumpString(0) << std::endl;

  return json;
}

void ZParameterDialog::resetValues()
{
  for (auto iter = m_parameterMap.constBegin();
       iter != m_parameterMap.constEnd(); ++iter) {
    const auto &p = iter.value();
    if (!p.defaultValue.isNull()) {
      setValue(iter.key(), p.defaultValue);
    }
  }
}

void ZParameterDialog::loadJsonObject(const ZJsonObject &obj)
{
  resetValues();

  obj.forEachValue([&](const std::string &key, ZJsonValue v) {
    QString name{key.c_str()};
    const Param &p = m_parameterMap.value(name);
    if (p.parameter) {
      if (p.valueType == "string") {
        setValue(name, ZString(v.toString()).trimmed().c_str());
      } else if (p.valueType == "int") {
        setValue(name, v.toInteger());
      } else if (p.valueType == "bool") {
        setValue(name, v.toBoolean());
      }
    }
  });
}
