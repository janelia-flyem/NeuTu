#ifndef FLYEMBODYANNOTATIONDIALOG_H
#define FLYEMBODYANNOTATIONDIALOG_H

#include <cstdint>
#include <string>
#include <functional>

#include <QDialog>
#include <QList>
#include <QSet>
#include <QMap>

class ZFlyEmBodyAnnotation;
class QLineEdit;
class QCheckBox;
class QComboBox;

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

  QVariant getValue(const QString &key) const;
  void setValue(const QString &key, const QVariant &value);

public:
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

private:
  void hideFinalizedStatus();
  void showFinalizedStatus();
  void freezeFinalizedStatus();
  void freezeUnknownStatus(const std::string &status);
  void processUnknownStatus(const std::string &status);
  void processUnknownProperty(const std::string &property);
  void initNullStatusItem();
  void initWidgetMap();

  enum class EWidgetType {
    LINE_EDIT, CHECK_BOX, COMBO_BOX
  };

  struct ValueManager {
    QWidget *m_widget = nullptr;
    EWidgetType m_type = EWidgetType::LINE_EDIT;
    std::function<QVariant()> m_getter;
    std::function<void(const QVariant&)> m_setter;

    ValueManager() {}
    ValueManager(QWidget *widget, EWidgetType type,
                 std::function<QVariant()> getter = nullptr,
                 std::function<void(const QVariant&)> setter = nullptr) :
      m_widget(widget), m_type(type), m_getter(getter), m_setter(setter) {}
  };

  void registerWidget(
      const QString &key, QWidget *widget, EWidgetType type,
      std::function<QVariant()> getter = nullptr,
      std::function<void(const QVariant&)> setter = nullptr);
  void registerWidget(const QString &key, QLineEdit *widget);
  void registerWidget(const QString &key, QComboBox *widget);
  void registerWidget(const QString &key, QCheckBox *widget);

  QWidget *getWidget(const QString &key) const;
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
  QSet<QString> m_adminStatusSet;
  QMap<QString, ValueManager> m_widgetMap;
};

#endif // FLYEMBODYANNOTATIONDIALOG_H
