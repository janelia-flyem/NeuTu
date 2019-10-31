#ifndef ZPARAMETER_H
#define ZPARAMETER_H

// The abstract base class for all parameters used in algorithms or renderers.
// A parameter can emit changed() signal while changed. And it has
// createWidget functions which can be used for UI
// generation. The changed() signal can be used to change algorithm
// or renderer behavior dynamicly.

//#include "QsLog.h"
#include "logging/zqslog.h"

#include <QObject>
#include <QStringList>
#include <QMutex>
#include <set>

class QWidget;

class QLayout;

class QLabel;

class ZParameter : public QObject
{
Q_OBJECT
public:
  explicit ZParameter(const QString& name, QObject* parent = nullptr);
  ~ZParameter();

  inline QString name() const
  { return m_name; }

  void setName(const QString& name);

  QString type() const;

  inline QString style() const
  { return m_style; }

  // if style does not exist, fall back to "DEFAULT"
  void setStyle(const QString& style);

  // create widget based on current style
  QLabel* createNameLabel(QWidget* parent = nullptr);

  QWidget* createWidget(QWidget* parent = nullptr);

  bool isVisible() const
  { return m_isWidgetsVisible; }

  bool isEnabled() const
  { return m_isWidgetsEnabled; }

  // set everything same as
  virtual void setSameAs(const ZParameter& rhs) = 0;

  // set value same as
  virtual void setValueSameAs(const ZParameter& rhs) = 0;

  // for option parameter
  virtual void forceSetValueSameAs(const ZParameter& rhs)
  { setValueSameAs(rhs); }

  inline bool isSameType(const ZParameter& rhs) const
  { return metaObject()->className() == rhs.metaObject()->className(); }

  void setVisible(bool s);

  void setEnabled(bool s);

  void updateFromSender();

//  void setWidgetSyncMutex(QMutex *m) { m_syncMutex = m; }

signals:

  void nameChanged(const QString&);

  void valueChanged();

  void setWidgetsEnabled(bool s);

  void setWidgetsVisible(bool s);

  // some templated subclass might need this
  void reservedIntSignal1(int);

  void reservedIntSignal2(int);

  void reservedStringSignal1(QString);

  void reservedStringSignal2(QString);

  void reservedSignal1();

  void reservedSignal2();

protected:
  // some templated subclass might need this
  virtual void reservedIntSlot1(int /*unused*/)
  {}

  virtual void reservedIntSlot2(int /*unused*/)
  {}

  virtual void reservedStringSlot1(QString /*unused*/)
  {}

  virtual void reservedStringSlot2(QString /*unused*/)
  {}

  virtual void reservedSlot1()
  {}

  virtual void reservedSlot2()
  {}

  inline void addStyle(const QString& style)
  { m_allStyles.push_back(style); }

  // all subclass should implement this function
  virtual QWidget* actualCreateWidget(QWidget* parent) = 0;

protected:
  QString m_name;
  QString m_style{"DEFAULT"};
  QStringList m_allStyles;

  //std::set<QWidget*> m_widgets;
  bool m_isWidgetsEnabled = true;
  bool m_isWidgetsVisible = true;
//  bool m_nameLabelNeeded = true;

//  QMutex *m_syncMutex = nullptr;
};

// parameter contains a single value
template<class T>
class ZSingleValueParameter : public ZParameter
{
public:
  ZSingleValueParameter(const QString& name, const T& value, QObject* parent = nullptr);

  explicit ZSingleValueParameter(const QString& name, QObject* parent = nullptr);

  void set(const T& valueIn);

  inline const T& get() const
  { return m_value; }

  inline T& get()
  { return m_value; }

  virtual void setValueSameAs(const ZParameter& rhs) override
  {
    CHECK(this->isSameType(rhs));
    set(static_cast<const ZSingleValueParameter<T>*>(&rhs)->get());
  }

protected:
  // subclass can use this function to change input value to a valid value
  // default implement do nothing
  virtual void makeValid(T& value) const;

  // subclass can use this function to emit customized signal before m_value
  // is changed or do other update, input is new value
  virtual void beforeChange(T& value);

  //
  virtual void afterChange(T& value);

protected:
  T m_value;
  bool m_locked = false;
};

//---------------------------------------------------------------------------

template<class T>
ZSingleValueParameter<T>::ZSingleValueParameter(const QString& name, const T& value, QObject* parent)
  : ZParameter(name, parent)
  , m_value(value)
{}

template<class T>
ZSingleValueParameter<T>::ZSingleValueParameter(const QString& name, QObject* parent)
  : ZParameter(name, parent)
{}

template<class T>
void ZSingleValueParameter<T>::set(const T& valueIn)
{
  if (m_locked)
    return;    // prevent widget change echo back
  if (m_value != valueIn) {
    T value = valueIn;
    makeValid(value);
    if (m_value != value) {
      m_locked = true;
      beforeChange(value);
      m_value = value;
      emit valueChanged();
      afterChange(value);
      m_locked = false;
    }
  }
}

template<class T>
void ZSingleValueParameter<T>::makeValid(T&/*value*/) const
{
}

template<class T>
void ZSingleValueParameter<T>::beforeChange(T&/*value*/)
{
}

template<class T>
void ZSingleValueParameter<T>::afterChange(T&/*value*/)
{
}

//-----------------------------------------------------------------------------------------------

class ZBoolParameter : public ZSingleValueParameter<bool>
{
Q_OBJECT
public:
  explicit ZBoolParameter(const QString& name, QObject* parent = nullptr);

  ZBoolParameter(const QString& name, bool value, QObject* parent = nullptr);

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

  void setValue(bool v);

signals:

  void valueWillChange(bool);

  void boolChanged(bool);

protected:
  virtual void beforeChange(bool& value) override;

  virtual void afterChange(bool& value) override;

  virtual QWidget* actualCreateWidget(QWidget* parent) override;
};

#endif // ZPARAMETER_H
