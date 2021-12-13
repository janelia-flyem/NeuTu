#include "zgenericbodyannotationdialog.h"

#include "logging/zlog.h"

ZGenericBodyAnnotationDialog::ZGenericBodyAnnotationDialog(QWidget *parent) :
  ZParameterDialog(parent)
{
  setWindowTitle("Body Annotation");
}

void ZGenericBodyAnnotationDialog::setDefaultStatusList(
    const QList<QString> statusList)
{
  m_defaultStatusList = statusList;
  updateStatusParameter();
}

void ZGenericBodyAnnotationDialog::updateStatusParameter()
{
  setParam("status", [&](Param &p) {
    p.defaultOptions = m_defaultStatusList;
    auto statusParam =
        qobject_cast<ZStringStringOptionParameter*>(getParameter("status"));
    if (statusParam) {
      updateOptions(statusParam, m_defaultStatusList, "");
    } else {
      addStringParameter("status", m_defaultStatusList, false);
    }
  }, true);

  updateGeneration();
}

void ZGenericBodyAnnotationDialog::addAdminStatus(const QString &status)
{
  m_adminStatusSet.insert(status);
}

void ZGenericBodyAnnotationDialog::configure(const ZJsonObject &config)
{
  ZParameterDialog::configure(config);
  if (!m_defaultStatusList.isEmpty()) {
    updateStatusParameter();
  }
}

void ZGenericBodyAnnotationDialog::build()
{
  ZParameterDialog::build();
}

void ZGenericBodyAnnotationDialog::setAdmin(bool on)
{
  m_isAdmin = on;
}

int ZGenericBodyAnnotationDialog::exec()
{
  if (m_adminStatusSet.contains(getStringValue("status"))) {
    auto param = getParameter("status");
    if (param) {
      param->setEnabled(m_isAdmin);
    } else {
      ZWARN(neutu::TOPIC_NULL) << "Unexpected null parameter.";
    }
  }

  return ZParameterDialog::exec();
}
