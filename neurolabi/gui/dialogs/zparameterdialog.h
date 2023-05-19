#ifndef ZPARAMETERDIALOG_H
#define ZPARAMETERDIALOG_H

#include <stdexcept>
#include <functional>

#include <QDialog>
#include <QMap>
#include <QList>
#include <QVBoxLayout>

#include "widgets/zparameter.h"
#include "widgets/zoptionparameter.h"

class ZJsonObject;

class ZParameterDialog : public QDialog
{
  Q_OBJECT
public:
  ZParameterDialog(QWidget *parent = nullptr);

  void addParameter(
      ZParameter *param, const QString &valueType, bool appending = true);
  void addStringParameter(const QString &name, bool appending = true);
  void addStringParameter(const QString &name, const QStringList &options, bool appending = true);
  void addIntParameter(const QString &name, int defaultValue, int min, int max);
  void addBoolParameter(const QString &name);

  void removeParameter(const QString &name);

  /*!
   * \brief Get the value of a given key.
   *
   * It throws an expection if value retrieval fails.
   */
  template<typename T>
  T getValue(const QString &name) const;

  QString getStringValue(const QString &name) const;
  int getIntValue(const QString &name) const;
  bool getBoolValue(const QString &name) const;

  template<typename T>
  void setValue(const QString &name, const T &value);

  void setValue(const QString &name, const QVariant &value);
  void setValue(const QString &name, const char *value);
  void setValue(const QString &name, const QString &value);

  template<typename T>
  void setDefaultValue(const QString &name, const T &value);

  virtual void configure(const ZJsonObject &config);
  virtual void build();

  ZJsonObject toJsonObject() const;
  void loadJsonObject(const ZJsonObject &obj);

  void resetValues();

  void setLabel(const QString &label, const QString &tooltip="");

  void addAuxWidget(QWidget *widget);

public slots:
  int exec() override;

protected:
  template<typename T>
  T getGenericValue(const QString &name) const;

  template<typename T>
  bool isDefaultValue(const QString &name, const T &value) const;

  bool buildRequied() const;

  ZParameter* getParameter(const QString &name) const;

  struct Param {
    ZParameter *parameter = nullptr;
    QString valueType;
    QVariant defaultValue;
    QStringList defaultOptions;
  };

  Param getParamBundle(const QString &name) const;
  Param setParam(
      const QString &name, std::function<void(Param&)> f, bool forcing);

  QStringList updateOptions(
      ZStringStringOptionParameter *param, const QStringList &options,
      const QString newOption);
//  void showEvent(QShowEvent *event) override;

  void updateGeneration();

  void resetParameterLayout();

  virtual void postProcess(ZJsonObject &obj) const;

private:
  QList<QString> m_shape;
  QMap<QString, Param> m_parameterMap;
  QVBoxLayout *m_mainLayout = nullptr;
  QVBoxLayout *m_parameterLayout = nullptr;
  QVBoxLayout *m_auxLayout = nullptr;
  QLabel *m_label = nullptr;
  int m_currentGeneration = 1;
  int m_prevGeneration = 0;
};

template<typename T>
T ZParameterDialog::getGenericValue(const QString &name) const
{
  ZSingleValueParameter<T> *param = dynamic_cast<ZSingleValueParameter<T>*>(
        getParameter(name));

  if (!param) {
    throw std::runtime_error("No matched parameter found");
  }

  return param->get();
}

template<typename T>
T ZParameterDialog::getValue(const QString &name) const
{
  return getGenericValue<T>(name);
}

template <typename T>
void ZParameterDialog::setValue(const QString &name, const T &value)
{
  ZSingleValueParameter<T> *param = dynamic_cast<ZSingleValueParameter<T>*>(
        getParameter(name));

  if (!param) {
    throw std::runtime_error("No matched parameter found");
  }

  param->set(value);
}

template <typename T>
bool ZParameterDialog::isDefaultValue(const QString &name, const T &value) const
{
  if (m_parameterMap.contains(name)) {
    const auto &defaultValue = m_parameterMap[name].defaultValue;
    if (defaultValue.isValid()) {
      return defaultValue == QVariant(value);
    }
  }

  return false;
}

template <typename T>
void ZParameterDialog::setDefaultValue(const QString &name, const T &value)
{
  if (m_parameterMap.contains(name)) {
    m_parameterMap[name].defaultValue.setValue(value);
  }
}

#endif // ZPARAMETERDIALOG_H
