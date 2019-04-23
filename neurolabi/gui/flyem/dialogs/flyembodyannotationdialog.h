#ifndef FLYEMBODYANNOTATIONDIALOG_H
#define FLYEMBODYANNOTATIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QSet>

#include <cstdint>

class ZFlyEmBodyAnnotation;

namespace Ui {
class FlyEmBodyAnnotationDialog;
}

class FlyEmBodyAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyAnnotationDialog(QWidget *parent = 0);
  ~FlyEmBodyAnnotationDialog();

  ZFlyEmBodyAnnotation getBodyAnnotation() const;

  void setBodyId(uint64_t bodyId);
  void setPrevUser(const std::string &name);
  void setPrevNamingUser(const std::string &name);

private:
  Ui::FlyEmBodyAnnotationDialog *ui;

  uint64_t m_bodyId;

  std::string m_oldName;

  QList<QString> m_defaultStatusList;
  QSet<QString> m_adminSatutsList;

  static const QString FINALIZED_TEXT;
};

#endif // FLYEMBODYANNOTATIONDIALOG_H
