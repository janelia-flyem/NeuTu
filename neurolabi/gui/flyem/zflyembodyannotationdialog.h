#ifndef ZFLYEMBODYANNOTATIONDIALOG_H
#define ZFLYEMBODYANNOTATIONDIALOG_H

#include <cstdint>

#include <QDialog>
#include <QSet>
#include <QList>

class ZFlyEmBodyAnnotation;

namespace Ui {
class ZFlyEmBodyAnnotationDialog;
}

class ZFlyEmBodyAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBodyAnnotationDialog(bool admin, QWidget *parent);
  ~ZFlyEmBodyAnnotationDialog();

  ZFlyEmBodyAnnotation getBodyAnnotation() const;

  void loadBodyAnnotation(const ZFlyEmBodyAnnotation &annotation);

  void setBodyId(uint64_t bodyId);
  void setPrevUser(const std::string &name);
  void setPrevNamingUser(const std::string &name);

  uint64_t getBodyId() const;
  QString getComment() const;
  QString getName() const;
  QString getType() const;
  QString getStatus() const;

  void setComment(const std::string &comment);
  void setStatus(const std::string &status);
  void setName(const std::string &name);
  void setType(const std::string &type);

  void hideFinalizedStatus();
  void showFinalizedStatus();
  void freezeFinalizedStatus();
  void freezeUnknownStatus(const std::string &status);
  void processUnknownStatus(const std::string &status);

  void setDefaultStatusList(const QList<QString> statusList);
  void addAdminStatus(const QString &status);
  void updateStatusBox();

public slots:
  void setNameEdit(const QString &name);

private:
  void connectSignalSlot();
  bool isNameChanged() const;

private:
  Ui::ZFlyEmBodyAnnotationDialog *ui;
  uint64_t m_bodyId;
  bool m_isAdmin = false;

  std::string m_oldName;

  QList<QString> m_defaultStatusList;
  QSet<QString> m_adminSatutsList;

  static const QString FINALIZED_TEXT;
};

#endif // ZFLYEMBODYANNOTATIONDIALOG_H
