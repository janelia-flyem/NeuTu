#ifndef FLYEMBODYSUPPLYWIDGET_H
#define FLYEMBODYSUPPLYWIDGET_H

#include <QPair>
#include <QWidget>
#include <QList>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>

class FlyEmBodySupplyWidget : public QWidget
{
  Q_OBJECT
public:
  explicit FlyEmBodySupplyWidget(QWidget *parent = nullptr);

  void setConfirmTitle(const QString &title);

signals:
  void confirmed(QList<uint64_t> bodyList);

private:
  void updateConfirmButton();
  void updateConfirmButton(const QString &bodyStr);

private slots:
  void processBodyEditChange(const QString &bodyStr);
  void confirm();
  QList<uint64_t> parseBodyList(const QString &bodystr);
  void clearEdit();
  void postConfirm(const QList<uint64_t> &bodyList);

private:
  QLineEdit *m_bodyEdit = nullptr;
  QPushButton *m_confirmButton = nullptr;
  QLayout *m_layout = nullptr;
  QPair<QString, QList<uint64_t>> m_bodyParsingCache;
  QString m_confirmTitle{"✓"};
  bool m_clearAfterConfirm = true;
};

#endif // FLYEMBODYSUPPLYWIDGET_H
