#ifndef ZPARAMETERARRAY_H
#define ZPARAMETERARRAY_H

#include <QVector>
#include<QPointer>
#include "zparameter.h"

class ZParameterArray
{
public:
  ZParameterArray();
  ~ZParameterArray();

  void append(ZParameter *param);
  const QVector<QPointer<ZParameter> >& getData() const;


private:
  QVector<QPointer<ZParameter> > m_paramArray;
};

#endif // ZPARAMETERARRAY_H
