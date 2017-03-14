#ifndef ZDVIDADVANCEDDIALOG_H
#define ZDVIDADVANCEDDIALOG_H

#include <QDialog>
#include <string>

#include "dvid/zdvidnode.h"

class ZDvidTarget;

namespace Ui {
class ZDvidAdvancedDialog;
}

class ZDvidAdvancedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidAdvancedDialog(QWidget *parent = 0);
  ~ZDvidAdvancedDialog();

  void update(const ZDvidTarget &target);
  void configure(ZDvidTarget *target);

  void setDvidServer(const QString &str);
  void updateWidgetForEdit(bool editable);
  void updateWidgetForDefaultSetting(bool usingDefault);

  void setTodoName(const std::string &name);
  std::string getTodoName() const;
  bool isSupervised() const;
  std::string getSupervisorServer() const;
  void setSupervised(bool supervised);
  void setSupervisorServer(const std::string &server);

  void setGrayscaleSource(const ZDvidNode &node);
  void setTileSource(const ZDvidNode &node);

  void setGrayscaleSource(const ZDvidNode &node, bool sameMainSource);
  void setTileSource(const ZDvidNode &node, bool sameMainSource);

  ZDvidNode getGrayscaleSource() const;
  ZDvidNode getTileSource() const;

  void backup();
  void recover();

private:
  Ui::ZDvidAdvancedDialog *ui;

  bool m_oldSupervised;
  std::string m_oldSupervisorServer;
  std::string m_oldTodoName;
  ZDvidNode m_oldGrayscaleSource;
  ZDvidNode m_oldTileSource;
  bool m_oldMainGrayscale;
  bool m_oldMainTile;
//  bool m_oldDefaultTodo;
};

#endif // ZDVIDADVANCEDDIALOG_H
