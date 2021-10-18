#ifndef ZGENERICBODYANNOTATIONDIALOG_H
#define ZGENERICBODYANNOTATIONDIALOG_H

#include "dialogs/zparameterdialog.h"

class ZGenericBodyAnnotationDialog : public ZParameterDialog
{
  Q_OBJECT
public:
  ZGenericBodyAnnotationDialog(QWidget *parent = nullptr);

  void configure(const ZJsonObject &config) override;
  void build() override;
  int exec() override;

  void setDefaultStatusList(const QList<QString> statusList);
  void addAdminStatus(const QString &status);
  void setAdmin(bool on);

private:
  void updateStatusParameter();

private:
  bool m_isAdmin = false;
  QList<QString> m_defaultStatusList;
  QSet<QString> m_adminStatusSet;
};

#endif // ZGENERICBODYANNOTATIONDIALOG_H
