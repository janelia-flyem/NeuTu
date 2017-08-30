#ifndef ZSTRINGPARAMETER_H
#define ZSTRINGPARAMETER_H

#include "zparameter.h"

class QLineEdit;

class ZStringParameter : public ZSingleValueParameter<QString>
{
Q_OBJECT
public:
  explicit ZStringParameter(const QString& name, QObject* parent = nullptr);

  explicit ZStringParameter(const QString& name, const QString& str, QObject* parent = nullptr);

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

signals:

  void stringChanged(QString str);

protected:
  void setContent(const QString& str);

  virtual QWidget* actualCreateWidget(QWidget* parent) override;

  virtual void afterChange(QString& value) override;
};

#endif // ZSTRINGPARAMETER_H
