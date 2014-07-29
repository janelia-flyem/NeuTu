#ifndef ZBUTTONBOX_H
#define ZBUTTONBOX_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

class ZButtonBox : public QWidget
{
  Q_OBJECT
public:
  explicit ZButtonBox(QWidget *parent = 0);

  typedef uint32_t TRole;
  const static TRole ROLE_NONE;
  const static TRole ROLE_YES;
  const static TRole ROLE_NO;
  const static TRole ROLE_CONTINUE;
  const static TRole ROLE_PAUSE;

  QPushButton* activate(TRole role);

signals:
  void clickedYes();
  void clickedNo();
  void clickedContinue();
  void clickedPause();

public slots:

private:
  QLayout *m_layout;
  QPushButton *m_yesButton;
  QPushButton *m_noButton;
  QPushButton *m_continueButton;
  QPushButton *m_paushButton;
};

#endif // ZBUTTONBOX_H
