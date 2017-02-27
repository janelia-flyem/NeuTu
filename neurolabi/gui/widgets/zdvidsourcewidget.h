#ifndef ZDVIDSOURCEWIDGET_H
#define ZDVIDSOURCEWIDGET_H

#include <QWidget>
#include <string>

class ZDvidNode;

namespace Ui {
class ZDvidSourceWidget;
}

class ZDvidSourceWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZDvidSourceWidget(QWidget *parent = 0);
  ~ZDvidSourceWidget();

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

  ZDvidNode getNode() const;

  void setNode(const ZDvidNode &node);
  void setAddress(const std::string &address);
  void setPort(int port);
  void setUuid(const std::string &uuid);

  void setReadOnly(bool readOnly);

private:
  Ui::ZDvidSourceWidget *ui;
};

#endif // ZDVIDSOURCEWIDGET_H
