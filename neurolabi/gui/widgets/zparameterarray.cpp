#include "zparameterarray.h"

ZParameterArray::ZParameterArray()
{
}

ZParameterArray::~ZParameterArray()
{
  foreach (QPointer<ZParameter> param, m_paramArray) {
    if (param) {
      if (param->parent() == NULL) { //not owned by any parent
        delete param.data();
      }
    }
  }
}

void ZParameterArray::append(ZParameter *param)
{
  if (param != NULL) {
    m_paramArray.append(param);
  }
}

const QVector<QPointer<ZParameter> >& ZParameterArray::getData() const
{
  return m_paramArray;
}
