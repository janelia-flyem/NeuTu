#ifndef ZCHECKBOXGROUP_H
#define ZCHECKBOXGROUP_H

#include <QWidget>
#include <QVector>
#include <functional>

#include "zgroupviswidget.h"

class QCheckBox;

/*!
 * \brief The widget of grouping a list of checkboxes
 */
class ZCheckBoxGroup : public ZGroupVisWidget
{
  Q_OBJECT
public:
  explicit ZCheckBoxGroup(QWidget* parent = nullptr);

  struct CheckBoxConfig {
    CheckBoxConfig(const QString &text, bool defaultValue,
                   QObject *receiver, const char *slot) :
      m_text(text), m_defaultValue(defaultValue), m_receiver(receiver),
      m_slot(slot) {}
    CheckBoxConfig() {}

    QString m_text;
    bool m_defaultValue = false;
    QObject *m_receiver = nullptr;
    const char *m_slot = nullptr;
  };

  struct CheckBoxConfigBuilder {
    CheckBoxConfigBuilder(const QString &text);

    operator CheckBoxConfig() const;
    CheckBoxConfigBuilder& connectTo(QObject *receiver, const char *slot);
    CheckBoxConfigBuilder& checkedByDefault(bool on);

  private:
    CheckBoxConfig m_config;
  };

  /*!
   * \brief Set a checkbox
   *
   * It set a checkbox according to \a config at \a index. The existing checkbox
   * at \a index will be deleted.
   */
  void setCheckBox(int index, const CheckBoxConfig &config);

  /*!
   * \brief Get the checkbox at a given index.
   *
   * \return The checkbox at \a index if it exists or nullptr.
   */
  QCheckBox* getCheckBox(int index) const;

  /*!
   * \brief Process a checkbox
   *
   * It applies a function \a f on the checkbox at \a index. \a f will not be
   * called if no checkbox exists at \a index.
   */
  void processCheckBox(int index, std::function<void(QCheckBox*)> f);


private:
  QVector<QCheckBox*> m_checkBoxList;
};

#endif // ZCHECKBOXGROUP_H
