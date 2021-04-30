#ifndef FLYEMBODYANNOTATIONDIALOG_H
#define FLYEMBODYANNOTATIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QSet>
#include <QMap>

#include <cstdint>
#include <string>

class ZFlyEmBodyAnnotation;

namespace Ui {
class FlyEmBodyAnnotationDialog;
}

class FlyEmBodyAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyAnnotationDialog(bool admin, QWidget *parent);
  ~FlyEmBodyAnnotationDialog();

  ZFlyEmBodyAnnotation getBodyAnnotation() const;
  void loadBodyAnnotation(const ZFlyEmBodyAnnotation &annotation);

  void setBodyId(uint64_t bodyId);
  void setPrevUser(const std::string &name);
  void setPrevNamingUser(const std::string &name);
  void setPrevStatusUser(const std::string &name);
  void setType(const std::string &type);
  void setInstance(const std::string &instance);
  void setComment(const std::string &comment);
  void setMajorInput(const std::string &v);
  void setMajorOutput(const std::string &v);
  void setPrimaryNeurite(const std::string &v);
  void setLocation(const std::string &v);
  void setOutOfBounds(bool v);
  void setCrossMidline(bool v);
  void setNeurotransmitter(const std::string &v);
  void setSynonym(const std::string &v);
  void setClonalUnit(const std::string &v);
  void setHemilineage(const std::string &v);
  void setAutoType(const std::string &v);
  void setProperty(const std::string &v);

  void setStatus(const std::string &status);

  uint64_t getBodyId() const;
  std::string getType() const;
  std::string getInstance() const;
  std::string getComment() const;
  std::string getMajorInput() const;
  std::string getMajorOutput() const;
  std::string getPrimaryNeurite() const;
  std::string getLocation() const;
  bool getOutOfBounds() const;
  bool getCrossMidline() const;
  std::string getNeurotransmitter() const;
  std::string getSynonym() const;
  std::string getStatus() const;
  std::string getProperty() const;
  std::string getAutoType() const;
  std::string getClonalUnit() const;
  std::string getHemilineage() const;
  bool isInstanceChanged() const;
  bool isStatusChanged() const;

  void setDefaultStatusList(const QList<QString> statusList);
  void addAdminStatus(const QString &status);
  void updateStatusBox();
  void updatePropertyBox();

private:
  void hideFinalizedStatus();
  void showFinalizedStatus();
  void freezeFinalizedStatus();
  void freezeUnknownStatus(const std::string &status);
  void processUnknownStatus(const std::string &status);
  void processUnknownProperty(const std::string &property);
  void initNullStatusItem();
  void initWidgetMap();

  void disableWidget(const QString &key);
  void hideWidget(const QString &key);

private slots:
  void fillType();

private:
  Ui::FlyEmBodyAnnotationDialog *ui;

  bool m_isAdmin = false;

  std::string m_oldInstance;
  std::string m_oldStatus;

  //Non-editable fields
  uint64_t m_bodyId = 0;
  std::string m_prevNamingUser;
  std::string m_prevStatusUser;

  QList<QString> m_defaultStatusList;
  QSet<QString> m_adminSatutsList;
  QMap<QString, QWidget*> m_widgetMap;

  static const QString FINALIZED_TEXT;
  static const QString KEY_TYPE;
  static const QString KEY_INSTANCE;
  static const QString KEY_COMMENT;
  static const QString KEY_MAJOR_INPUT;
  static const QString KEY_MAJOR_OUTPUT;
  static const QString KEY_PRIMARY_NEURITE;
  static const QString KEY_LOCATION;
  static const QString KEY_OUT_OF_BOUNDS;
  static const QString KEY_CROSS_MIDLINE;
  static const QString KEY_NEUROTRANSMITTER;
  static const QString KEY_SYNONYM;
  static const QString KEY_CLONAL_UNIT;
  static const QString KEY_HEMILINEAGE;
  static const QString KEY_AUTO_TYPE;
  static const QString KEY_PROPERTY;
  static const QString KEY_STATUS;
};

#endif // FLYEMBODYANNOTATIONDIALOG_H
