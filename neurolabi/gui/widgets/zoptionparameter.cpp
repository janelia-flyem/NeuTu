#include "zoptionparameter.h"

#include "zutils.h"

template<class T, class T2>
ZOptionParameter<T, T2>::ZOptionParameter(const QString& name, QObject* parent, const QString& prefix,
                                          const QString& suffix)
  : ZSingleValueParameter<T>(name, parent)
  , m_prefix(prefix), m_suffix(suffix)
{
}

template<class T, class T2>
void ZOptionParameter<T, T2>::select(const T& value)
{
  this->set(value);
}

template<class T, class T2>
void ZOptionParameter<T, T2>::selectNext()
{
  if (m_options.size() < 2)
    return;
  if (!m_dataIsValid) {
    select(m_options[0]);
    m_dataIsValid = true;
  } else {
    int index = m_options.indexOf(this->m_value);
    CHECK(index >= 0);
    if (++index >= m_options.size())
      index = 0;
    select(m_options[index]);
  }
}

template<class T, class T2>
bool ZOptionParameter<T, T2>::isSelected(const T& value) const
{
#if defined(_DEBUG_)
  if (!m_options.contains(value)) {
    LOG(ERROR) << QString("Option <%1> does not exist.").arg(value);
  }
#endif
  return value == this->m_value;
}

template<class T, class T2>
void ZOptionParameter<T, T2>::reservedIntSlot1(int index)
{
  // notify all widgets
  if (index >= 0 && index < m_options.size())
    this->set(m_options[index]);
}

template<class T, class T2>
QWidget* ZOptionParameter<T, T2>::actualCreateWidget(QWidget* parent)
{
  auto cb = new ZComboBox(parent);

  for (int i = 0; i < m_options.size(); ++i) {
    cb->addItem(comboBoxItemString(m_options[i]));
  }
  if (!m_options.empty()) {
    int index = m_options.indexOf(this->m_value);
    if (index != -1) {
      cb->setCurrentIndex(index);
    } else {
      cb->setCurrentIndex(0);
      this->set(m_options[0]);
    }
  }
  this->connect(this, &ZOptionParameter::reservedIntSignal1, cb, &ZComboBox::setCurrentIndex);
  this->connect(this, &ZOptionParameter::reservedStringSignal1, cb, &ZComboBox::addItemSlot);
  this->connect(this, &ZOptionParameter::reservedStringSignal2, cb, &ZComboBox::removeItemSlot);
  this->connect(this, &ZOptionParameter::reservedSignal1, cb, &ZComboBox::clear);
#if __cplusplus > 201103L
  this->connect(cb, qOverload<int>(&ZComboBox::currentIndexChanged), this, &ZOptionParameter::reservedIntSlot1);
#else
  this->connect(cb, QOverload<int>::of(&ZComboBox::currentIndexChanged), this, &ZOptionParameter::reservedIntSlot1);
#endif
  return cb;
}

template<class T, class T2>
void ZOptionParameter<T, T2>::makeValid(T& value) const
{
  if (!m_options.contains(value)) {
    LOG(ERROR) << QString("Optiong value <%1> does not exist.").arg(value);
    if (m_options.empty())
      LOG(ERROR) << QString("Error: Try to select <%1> from empty options list. Call addOptions() first!").arg(value);
    else if (m_dataIsValid) {
      LOG(ERROR) << QString("Warning: Select failed, value is still <%1>").arg(this->m_value);
      value = this->m_value;
    } else {
      LOG(ERROR) << QString("Default to first option <%1>").arg(m_options[0]);
      value = m_options[0];
    }
  }
}

template<class T, class T2>
QString ZOptionParameter<T, T2>::comboBoxItemString(const T& value) const
{
  return QString("%1%2%3").arg(m_prefix).arg(value).arg(m_suffix);
}

template<class T, class T2>
void ZOptionParameter<T, T2>::beforeChange(T& value)
{
  int index = m_options.indexOf(value);
  m_associatedData = m_associatedDatas[index];
  emit this->reservedIntSignal1(index);
}

template<class T, class T2>
void ZOptionParameter<T, T2>::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const ZOptionParameter<T, T2>* src = static_cast<const ZOptionParameter<T, T2>*>(&rhs);
  m_prefix = src->m_prefix;
  m_suffix = src->m_suffix;
  m_dataIsValid = src->m_dataIsValid;
  if (m_options != src->m_options || m_associatedDatas != src->m_associatedDatas) {
    clearOptions();
    for (int i = 0; i < src->m_options.size(); ++i) {
      addOptionWithData(qMakePair(src->m_options[i], src->m_associatedDatas[i]));
    }
  }
  this->set(src->get());
  ZParameter::setSameAs(rhs);
}

template<class T, class T2>
void ZOptionParameter<T, T2>::forceSetValueSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const ZOptionParameter<T, T2>* src = static_cast<const ZOptionParameter<T, T2>*>(&rhs);
  if (hasOption(src->get())) {
    select(src->get());
  } else {
    addOptionWithData(qMakePair(src->get(), src->associatedData()));
    select(src->get());
  }
}

template
class ZOptionParameter<QString, int>;

template
class ZOptionParameter<int, int>;

template
class ZOptionParameter<QString, QString>;

ZStringIntOptionParameter::ZStringIntOptionParameter(const QString& name, QObject* parent, const QString& prefix,
                                                     const QString& suffix)
  : ZOptionParameter<QString, int>(name, parent, prefix, suffix)
{
}


ZStringStringOptionParameter::ZStringStringOptionParameter(const QString& name, QObject* parent, const QString& prefix,
                                                           const QString& suffix)
  : ZOptionParameter<QString, QString>(name, parent, prefix, suffix)
{
}


ZIntIntOptionParameter::ZIntIntOptionParameter(const QString& name, QObject* parent, const QString& prefix,
                                               const QString& suffix)
  : ZOptionParameter<int, int>(name, parent, prefix, suffix)
{
}
