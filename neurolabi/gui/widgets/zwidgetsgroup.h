#ifndef ZWIDGETSGROUP_H
#define ZWIDGETSGROUP_H

#include <QObject>
#include <QLayout>
#include <memory>

class QLabel;

class ZParameter;

class ZWidgetsGroup : public QObject
{
Q_OBJECT
public:
  enum class Type
  {
    Group, Widget, Parameter
  };

  explicit ZWidgetsGroup(QWidget& widget, int visibleLevel);

  explicit ZWidgetsGroup(const QString& groupName, int visibleLevel);

  explicit ZWidgetsGroup(ZParameter& parameter, int visibleLevel);

  inline bool isGroup() const
  { return m_type == Type::Group; }

  inline QString getGroupName() const
  { return m_groupName; }

  inline void setGroupName(const QString& name)
  { m_groupName = name; }

  inline void setUseToolBoxStyle(bool v)
  { m_useToolBoxStyle = v; }

  std::vector<ZParameter*> getParameterList();

  const std::vector<std::shared_ptr<ZWidgetsGroup>>& getChildGroups();

  void addChild(QWidget& widget, int visibleLevel);

  void addChild(ZParameter& parameter, int visibleLevel);

  void addChild(std::shared_ptr<ZWidgetsGroup> child, bool atEnd = true);

  void removeAllChildren();

  void removeChild(const ZParameter& para);

  void removeChild(const std::shared_ptr<ZWidgetsGroup>& child);

  inline int visibleLevel() const
  { return m_visibleLevel; }

  inline void setVisibleLevel(int v)
  { m_visibleLevel = v; }

  inline void setBasicAdvancedCutoff(int v)
  { m_cutOffbetweenBasicAndAdvancedLevel = v; }

  inline int basicAdvancedCutoff() const
  { return m_cutOffbetweenBasicAndAdvancedLevel; }

  void setVisible(bool visible)
  { m_isVisible = visible; }

  inline bool isVisible()
  { return m_isVisible; }

  QWidget* createWidget(bool createBasic = true, bool scroll = true, QLabel* label = nullptr);

  QLayout* createLayout(bool createBasic = true);

  bool operator<(const ZWidgetsGroup& other) const;

  void emitRequestAdvancedWidgetSignal();

  void emitWidgetsGroupChangedSignal();

signals:

  void requestAdvancedWidget(const QString& name);

  void widgetsGroupChanged();

private:
  void sortChildGroups();
  QLayout* createWidgetLayout();
  QLayout* createParameterLayout();
  QLayout* createGroupLayout(bool createBasic);

private:
  Type m_type;
  QString m_groupName;
  QWidget* m_widget = nullptr;
  ZParameter* m_parameter = nullptr;
  int m_visibleLevel;
  bool m_isSorted = false;
  int m_cutOffbetweenBasicAndAdvancedLevel = 100;
  std::vector<std::shared_ptr<ZWidgetsGroup>> m_childGroups;
  bool m_isVisible = true;

  bool m_useToolBoxStyle = false;
};

#endif // ZWIDGETSGROUP_H
